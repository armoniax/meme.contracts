#pragma once

#include <eosio/asset.hpp>
#include <eosio/action.hpp>

#include <string>

#include <applylisting/applylisting.db.hpp>
#include <wasm_db.hpp>

namespace fufi {

using std::string;
using std::vector;
         
using namespace wasm::db;
using namespace eosio;

static constexpr name      NFT_BANK    = "did.ntoken"_n;
static constexpr eosio::name active_perm{"active"_n};


enum class err: uint8_t {
   NONE                 = 0,
   RECORD_NOT_FOUND     = 1,
   RECORD_EXISTING      = 2,
   SYMBOL_MISMATCH      = 4,
   PARAM_ERROR          = 5,
   MEMO_FORMAT_ERROR    = 6,
   PAUSED               = 7,
   NO_AUTH              = 8,
   NOT_POSITIVE         = 9,
   NOT_STARTED          = 10,
   OVERSIZED            = 11,
   TIME_EXPIRED         = 12,
   NOTIFY_UNRELATED     = 13,
   ACTION_REDUNDANT     = 14,
   ACCOUNT_INVALID      = 15,
   FEE_INSUFFICIENT     = 16,
   FIRST_CREATOR        = 17,
   STATUS_ERROR         = 18,
   SCORE_NOT_ENOUGH     = 19,
   NEED_REQUIRED_CHECK  = 20

};

/**
 * The `applylisting` sample system contract defines the structures and actions that allow users to create, issue, and manage tokens for AMAX based blockchains. It demonstrates one way to implement a smart contract which allows for creation and management of tokens. It is possible for one to create a similar contract which suits different needs. However, it is recommended that if one only needs a token with the below listed actions, that one uses the `applylisting` contract instead of developing their own.
 *
 * The `applylisting` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
 *
 * The `applylisting` contract manages the set of tokens, accounts and their corresponding balances, by using two internal multi-index structures: the `accounts` and `stats`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
 *
 * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, and the creator account for a symbol token. The `stats` table is scoped to the token symbol.  Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
 */
class [[eosio::contract("applylisting")]] applylisting : public contract {
   
   private:
      dbc                 _dbc;
   public:
      using contract::contract;
  
   applylisting(eosio::name receiver, eosio::name code, datastream<const char*> ds): contract(receiver, code, ds),
         _dbc(get_self()),
         _global(get_self(), get_self().value),
         _apply_tbl(get_self(), get_self().value)
    {
        _gstate = _global.exists() ? _global.get() : global_t{};
        
    }

    ~applylisting() { _global.set( _gstate, get_self() ); }


   ACTION init(const name& admin);

   [[eosio::on_notify("*::transfer")]]
   void on_transfer(const name& from, const name& to, const asset& quantity, const string& memo);

   ACTION apply(
           const name&                    submiter,
         const name&                    requester,          //sequence
         const extended_asset&          total_supply,       //PK
         const extended_symbol&         quote_coin,         //交易对买symbol
         const string&                  description,        //描述
         const string&                  icon_url,           //logo
         const string&                  media_urls,   
         const string&                  whitepaper_url,     //白皮书
         const string&                  issue_at,           //发行时间
         const uint64_t&                issue_price,        //发行价格 1/亿
         const uint64_t&                init_price          //初始价格 1/亿
    );

  

   private:
      global_singleton     _global;
      global_t             _gstate;
      apply_t::table       _apply_tbl;
      inline static name _get_tpcode( const symbol& base_symbol, const symbol& quote_symbol ) {
         auto str = ( base_symbol.code().to_string() + "." + quote_symbol.code().to_string() );
         std::transform(str.begin(), str.end(),str.begin(), ::tolower);
         return name(str);
      }
};
} //namespace amax
