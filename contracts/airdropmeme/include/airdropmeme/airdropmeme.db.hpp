#pragma once

#include <eosio/asset.hpp>
#include <eosio/privileged.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/binary_extension.hpp> 
#include <utils.hpp>

#include <optional>
#include <string>
#include <map>
#include <set>
#include <type_traits>


namespace meme {

using namespace std;
using namespace eosio;

#define HASH256(str) sha256(const_cast<char*>(str.c_str()), str.size())

static constexpr uint32_t MAX_LOGO_SIZE        = 512;
static constexpr uint32_t MAX_TITLE_SIZE        = 2048;

#define TBL struct [[eosio::table, eosio::contract("airdropmeme")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("airdropmeme")]]

NTBL("global") global_t {
    name            admin;   
    name            applynewmeme_contract;
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

//scope: _self
TBL airdrop_t {
    name                    owner;              //sequence
    extended_asset          quant;             //total supply
    name                    status;             //状态  enable disable
    time_point_sec          created_at;
    time_point_sec          updated_at;
    airdrop_t() {}
    airdrop_t(const name& i): owner(i) {}

    uint64_t primary_key()const { return quant.quantity.symbol.code().raw() ; }

    typedef eosio::multi_index< "airdrops"_n,  airdrop_t> table;

    EOSLIB_SERIALIZE( airdrop_t,   (owner)(quant)(status)(created_at)(updated_at))
};


} //namespace amax