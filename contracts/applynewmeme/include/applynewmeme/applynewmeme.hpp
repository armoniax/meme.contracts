#pragma once

#include <eosio/asset.hpp>
#include <eosio/action.hpp>

#include <string>

#include <applynewmeme/applynewmeme.db.hpp>
#include <wasm_db.hpp>

namespace meme {

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
 * The `applynewmeme` sample system contract defines the structures and actions that allow users to create, issue, and manage tokens for AMAX based blockchains. It demonstrates one way to implement a smart contract which allows for creation and management of tokens. It is possible for one to create a similar contract which suits different needs. However, it is recommended that if one only needs a token with the below listed actions, that one uses the `applynewmeme` contract instead of developing their own.
 *
 * The `applynewmeme` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
 *
 * The `applynewmeme` contract manages the set of tokens, accounts and their corresponding balances, by using two internal multi-index structures: the `accounts` and `stats`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
 *
 * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, and the creator account for a symbol token. The `stats` table is scoped to the token symbol.  Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
 */
class [[eosio::contract("applynewmeme")]] applynewmeme : public contract {
   
   private:
      dbc                 _dbc;
   public:
      using contract::contract;
  
   applynewmeme(eosio::name receiver, eosio::name code, datastream<const char*> ds): contract(receiver, code, ds),
         _dbc(get_self()),
         _global(get_self(), get_self().value),
         _meme_tbl(get_self(), get_self().value)
    {
        _gstate = _global.exists() ? _global.get() : global_t{};
    }

    ~applynewmeme() { _global.set( _gstate, get_self() ); }

   ACTION init(const name& admin, 
               const name& airdrop_contract, 
               const name& swap_contract, 
               const name& dex_apply_contract, 
               const name& meme_token_contract);

   [[eosio::on_notify("*::transfer")]]
   void on_transfer(const name& from, const name& to, const asset& quantity, const string& memo);

   ACTION clearmeme(const symbol& symbol);
   ACTION applymeme(
            const name&             applicant, 
            const asset&            meme_coin,
            const string&           coin_name,
            const extended_asset&   quote_coin, //交易对买symbol MUSDT, AMAX, MUSE
            const string&           description,
            const string&           icon_url, 
            const string&           media_urls,  //twitter, telegram, descriptionord
            const string&           whitepaper_url,
            const bool&             airdrop_mode_on,
            const uint64_t&         airdrop_ratio,
            const uint64_t&         fee_ratio,           //转账手续费销毁
            const uint64_t&         swap_sell_fee_ratio,
            const name&             swap_sell_fee_receiver, //转账手续费接收账户
            const string&           issue_at
   );  

   ACTION closeairdrop(const symbol& symbol);

   ACTION updatemedia(const symbol& symbol, const string& media_urls);

   ACTION applytruedex(const symbol& symbol);

   ACTION addmcap(const symbol& symbol, const asset& threshold);

   private:
      void _hootswap_create(const extended_asset& sell_ex_quant, const extended_asset& buy_ex_quant,
                        const int16_t& swap_sell_fee_ratio, const name& swap_sell_fee_receiver);

      uint64_t _rand(const name& user, const uint64_t& range);

      asset _get_current_market_value(const extended_symbol& buy_symbol, 
                                 const extended_symbol& sell_symbol, 
                                 uint64_t& current_price);
      global_singleton     _global;
      global_t             _gstate;
      meme_t::table        _meme_tbl;
};
} //namespace amax
