#include <addressbookt/addressbookt.hpp>

using namespace eosio;
using namespace std;
using std::string;

//account: mgp.ecoshare
namespace mgp {

void smart_mgp::transfer(name from, name to, asset quantity, string memo){
	require_auth( from );
	if (to != _self) return;
	
	check( quantity.symbol.is_valid(), "Invalid quantity symbol name" );
	check( quantity.is_valid(), "Invalid quantity");
	check( quantity.amount > 0, "Amount quantity must be more then 0" );
	check( quantity.symbol == SYS_SYMBOL, "Token Symbol not allowed" );
	check( quantity >= _gstate.minpay, "Should stake more than [" + _gstate.minpay.to_string() + "]" );
	
	name orderAccount = from;
	
	asset to_burn;
	to_burn.amount = ( quantity.amount / 100 ) * _gstate.destruction;
	to_burn.symbol = quantity.symbol;

	asset remaining;
	remaining.symbol = quantity.symbol;
	
	if (from == SYS_ACCOUNT){
		orderAccount = name(memo);
		remaining.amount = quantity.amount;

	} else if (from == SHOP_ACCOUNT || from == AGENT_ACCOUNT) {
		orderAccount = name(memo);
		remaining.amount = quantity.amount - to_burn.amount;

	} else {
		remaining.amount = quantity.amount - to_burn.amount;
	}

	balances_t bal( orderAccount );
	if( !_dbc.get(bal) ) {
		bal.account = orderAccount;
		bal.remaining = remaining;

	} else {
		bal.remaining.amount += remaining.amount;
	}

	_dbc.set( bal );

	/** 
	 * FIXME: once eosio.token upgraded, use burn
	 * 
	 *  Note: probably not to use burn due to data issues
	 * 
	 */ 
	if (from != SYS_ACCOUNT) {
		action(
			permission_level{ _self, "active"_n }, SYS_BANK, "transfer"_n,
			std::make_tuple( _self, "eosio.token"_n, to_burn, _gstate.burn_memo)
		).send();

		// action(
		// 	permission_level{ _self, "active"_n },	SYS_BANK, "burn"_n,
		// 	std::make_tuple( from, to_burn, "staking burn")
		// ).send();
	}
}

[[eosio::action]]
void smart_mgp::configure( string burn_memo, int destruction, bool redeemallow, asset minpay ){
	require_auth(get_self());
	
	check( _gstate.account == _self, "configured account is not " + _self.to_string() );

	_gstate.burn_memo 	= burn_memo;
	_gstate.destruction = destruction;
	_gstate.redeemallow = redeemallow;
	_gstate.minpay 		= minpay;
}


[[eosio::action]]
void smart_mgp::encorrection( bool enable_data_correction ) {
	require_auth( _self );

	_gstate2.data_correction_enabled = enable_data_correction;

}


[[eosio::action]]
void smart_mgp::bindaddress(const name& account, const string& address) {
    require_auth(account);

	check(address != "", "Address is empty");
	check(address.size() < 64, "Eth address oversized!");

    ethaddressbook_t book(account);
	check( !_dbc.get(book), "Account (" + account.to_string() + ") was already bound" );

	book.account = account;
	book.address = address;

	_dbc.set(book);
}

[[eosio::action]]
void smart_mgp::delbind(const name& account, const string& address) {
    require_auth( account );

    ethaddressbook_t book(account);
    check( _dbc.get(book), "Account (" + account.to_string() + ") not yet bound" );
    _dbc.del(book);
	
}


[[eosio::action]]
void smart_mgp::redeem(const name& issuer){
	require_auth( issuer );
	
	check( _gstate.redeemallow, "Redeem not allowed!" );

	balances_t bal(issuer);
	check( _dbc.get(bal), "Balance not found!");

	if (bal.remaining.amount > 0) {
		action(
			permission_level{ get_self(), "active"_n }, SYS_BANK, "transfer"_n,
			std::make_tuple( get_self(), issuer, bal.remaining, std::string(""))
		).send();
	}

	bal.remaining.amount = 0;

	_dbc.set(bal);
}

/**
 * below is added to correct numbers due to historical bugs
 * 
 * this function shall be suppressed unless ultimately necessary for data cleaning purposes
 */
void smart_mgp::reloadnum(const name& from, const name& to, const asset& quant) {
	require_auth( _self );
	
	check( _gstate2.data_correction_enabled, "data correction disabled" );

	balances_t from_bal( from );
	check( _dbc.get(from_bal), "from balance not found!" );

	balances_t to_bal( to );
	check( _dbc.get(to_bal), "to balance not found!" );
			
	check( from_bal.remaining >= quant, "from balance: ("+ from_bal.remaining.to_string() 
										+  "overdrawn" );

	from_bal.remaining.amount -= quant.amount;
	to_bal.remaining.amount += quant.amount;

	_dbc.set(from_bal);
	_dbc.set(to_bal);
}

} //end of namespace:: mgpecoshare