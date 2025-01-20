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

#define TBL struct [[eosio::table, eosio::contract("applynewmeme")]]
#define NTBL(name) struct [[eosio::table(name), eosio::contract("applynewmeme")]]

/* public key -> update content */
typedef std::variant<eosio::public_key, string> recover_target_type;

NTBL("global") global_t {
    name                     admin               = "armoniaadmin"_n;             //管理员
    name                     airdrop_contract    = "memesairdrop"_n;            //空投合约
    name                     swap_contract       = "hootswappool"_n;            //hootswap合约
    name                     dex_apply_contract  = "applylisting"_n;              //tyche申请合约:
    name                     meme_token_contract = "meme.token"_n;              //meme token合约
    map<symbol, asset>       mcap_list_threshold;       //truedex请求条件 eg: [8,AMAX->10000 AMAX]
                                                        //                   [6,USDT->10000 USDT]
                                                        //                   [8,MUSE->10000 MUSE]

    EOSLIB_SERIALIZE( global_t, (admin)(airdrop_contract)(swap_contract)
                                (dex_apply_contract)(meme_token_contract)(mcap_list_threshold))
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

//scope: _self
TBL meme_t {
    name                    applicant;                          //sequence
    extended_asset          total_supply;                       //PK
    string                  coin_name;                        //symbol name
    extended_asset          quote_coin;                         //交易对买symbol MUSDT, AMAX, MUSE
    string                  description;
    string                  icon_url;                           //logo
    string                  media_urls;
    string                  whitepaper_url;    
    uint64_t                airdrop_ratio;                      //空投比例
    uint64_t                fee_ratio;                          //转账手续费销毁,转账手续费就是销毁
    uint64_t                swap_sell_fee_ratio;                //转账手续费比例
    name                    swap_sell_fee_receiver = "oooo"_n;  //转账手续费接收账
    bool                    airdrop_enable;                     //是否开启空投
    string                  issue_at;                           //发行时间
    name                    status;                             //状态  enable disable
    time_point_sec          created_at;
    meme_t() {}
    meme_t(const name& i): applicant(i) {}

    uint64_t primary_key()const { return total_supply.quantity.symbol.code().raw() ; }

    typedef eosio::multi_index< "memelist"_n,  meme_t> table;

    EOSLIB_SERIALIZE( meme_t,  (applicant)(total_supply)(coin_name)(quote_coin)(description)(icon_url)(media_urls)(whitepaper_url)
                                (airdrop_ratio)(fee_ratio)(swap_sell_fee_ratio)(swap_sell_fee_receiver)
                                (airdrop_enable)(issue_at)(status)(created_at))
};


} //namespace amax