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

namespace ProducerStatus {
    static constexpr eosio::name DISABLE     { "disable"_n   };
    static constexpr eosio::name ENABLE      { "enable"_n  };
}

namespace ActionType {
    static constexpr eosio::name APPLYBP      { "newaccount"_n  }; //create a new account
    static constexpr eosio::name BINDINFO       { "bindinfo"_n    }; //bind audit info
    static constexpr eosio::name UPDATEINFO     { "updateinfo"_n    }; //bind audit info
    static constexpr eosio::name CREATECORDER   { "createorder"_n }; //create recover order
    static constexpr eosio::name SETSCORE       { "setscore"_n    }; //set score for user verfication
}

/* public key -> update content */
typedef std::variant<eosio::public_key, string> recover_target_type;

NTBL("global") global_t {
    name                     admin;   
    name                     airdrop_contract;      //空投合约
    name                     swap_contract;         //hootswap合约
    name                     fufi_contract;         //fufi合约
    name                     fufi_apply_contract;   //fufi申请合约
    name                     meme_token_contract;   //meme token合约

    EOSLIB_SERIALIZE( global_t, (admin)(airdrop_contract)(swap_contract)(fufi_contract)(fufi_apply_contract)(meme_token_contract))
};

typedef eosio::singleton< "global"_n, global_t > global_singleton;

//scope: _self
TBL meme_t {
    name                    owner;              //sequence
    extended_asset          total_supply;               //PK
    string                  disc;
    string                  icon_url;           //logo
    string                  urls;    
    uint64_t                airdrop_ratio;      //空投比例
    uint64_t                destroy_ratio;      //转账手续费销毁
    uint64_t                transfer_ratio;     //转账手续费比例
    name                    fee_receiver;       //转账手续费接收账户
    bool                    airdrop_enable;     //是否开启空投
    extended_symbol         trade_symbol;       //交易对买symbol
    uint64_t                init_price;        
    name                    status;             //状态  enable disable
    time_point_sec          created_at;
    time_point_sec          updated_at;
    meme_t() {}
    meme_t(const name& i): owner(i) {}

    uint64_t primary_key()const { return total_supply.quantity.symbol.code().raw() ; }

    typedef eosio::multi_index< "memes"_n,  meme_t> table;

    EOSLIB_SERIALIZE( meme_t,   (owner)(total_supply)(disc)(icon_url)(urls)(airdrop_ratio)(destroy_ratio)(transfer_ratio)
                            (fee_receiver)(airdrop_enable)(trade_symbol)(init_price)(status)(created_at)(updated_at))
};


} //namespace amax