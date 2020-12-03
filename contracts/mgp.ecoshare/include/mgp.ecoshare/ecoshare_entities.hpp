 #pragma once

#include <eosio/asset.hpp>
#include <eosio/privileged.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>

#include <deque>
#include <optional>
#include <string>
#include <type_traits>

namespace mgp {

using namespace eosio;

static constexpr uint64_t ECOSHARE_SCOPE = 1000;

#define CONTRACT_TBL [[eosio::table, eosio::contract("mgp.ecoshare")]]

struct [[eosio::table("global"), eosio::contract("mgp.ecoshare")]] global_tbl {
    uint64_t bps_voting_share = 2000; //20% * 10000
    name bps_voting_account = "mgp.bpsvoting"_n;
    name stake_mining_account = "addressbookt"_n;

    global_tbl(){}

    EOSLIB_SERIALIZE( global_tbl, (bps_voting_share)(bps_voting_account)(stake_mining_account) )
};
typedef eosio::singleton< "global"_n, global_tbl > global_singleton;


struct CONTRACT_TBL transfer_t {
    uint64_t id;

    name bps_voting_account;
    name stake_mining_account;
    asset bps_voting_share;
    asset stake_mining_share;

    transfer_t() {}
    transfer_t(name code, uint64_t scope) {
        index_t tbl(code, scope);
        id = tbl.available_primary_key();
    }
    transfer_t(const uint64_t tid): id(tid){}

    uint64_t primary_key() const { return id; }
    uint64_t scope() const { return 0; }
    
    typedef eosio::multi_index<"transfers"_n, transfer_t> index_t;

    EOSLIB_SERIALIZE( transfer_t,   (id)(bps_voting_account)(stake_mining_account)
                                    (bps_voting_share)(stake_mining_share) )
};


}