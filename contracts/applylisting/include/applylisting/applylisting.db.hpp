#pragma once

#include <eosio/asset.hpp>
#include <eosio/privileged.hpp>
#include <eosio/singleton.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <utils.hpp>

#include <optional>
#include <string>
#include <map>
#include <set>
#include <type_traits>


namespace fufi {

using namespace std;
using namespace eosio;

#define HASH256(str) sha256(const_cast<char*>(str.c_str()), str.size())

static constexpr uint32_t MAX_LOGO_SIZE        = 512;
static constexpr uint32_t MAX_TITLE_SIZE        = 2048;

#define TBL struct [[eosio::table, eosio::contract("applylisting")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("applylisting")]]

NTBL("global") global_t {
    name                     admin;   
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

//scope: _self
TBL apply_t {
    name                    submiter;
    name                    tpcode;             //PK
    name                    requester;          //sequence
    extended_asset          total_supply;       //PK
    extended_symbol         quote_coin;         //交易对买symbol
    string                  description;        //描述
    string                  icon_url;           //logo
    string                  media_urls;    
    string                  whitepaper_url;     //白皮书
    name                    status;             //状态  init, paid, audit, pass, reject
    string                  issue_at;           //发行时间
    uint64_t                issue_price;        //发行价格
    uint64_t                init_price;         //初始价格

    extended_asset          paid_quant; 
    time_point_sec          created_at;
    time_point_sec          updated_at;
    apply_t() {}

    uint64_t primary_key()const { return tpcode.value; }

    typedef eosio::multi_index< "applys"_n,  apply_t> table;

    EOSLIB_SERIALIZE( apply_t, (submiter)(tpcode)(requester)(total_supply)(quote_coin)(description)(icon_url)(media_urls)(whitepaper_url)(status)(issue_at)(issue_price)(init_price)(paid_quant)(created_at)(updated_at) )
};


} //namespace amax