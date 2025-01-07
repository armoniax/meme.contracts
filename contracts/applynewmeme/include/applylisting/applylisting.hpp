#pragma once

#include <eosio/asset.hpp>
#include <eosio/action.hpp>

#include <string>

#include <wasm_db.hpp>

namespace tyche {

using std::string;
using std::vector;
         
using namespace wasm::db;
using namespace eosio;

/**
 * The `applylisting` sample system contract defines the structures and actions that allow users to create, issue, and manage tokens for AMAX based blockchains. It demonstrates one way to implement a smart contract which allows for creation and management of tokens. It is possible for one to create a similar contract which suits different needs. However, it is recommended that if one only needs a token with the below listed actions, that one uses the `applylisting` contract instead of developing their own.
 *
 * The `applylisting` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
 *
 * The `applylisting` contract manages the set of tokens, accounts and their corresponding balances, by using two internal multi-index structures: the `accounts` and `stats`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
 *
 * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, and the creator account for a symbol token. The `stats` table is scoped to the token symbol.  Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
 */
class applylisting {
   
   public:
      ACTION apply(
         const name&                    requester,          //sequence
         const extended_asset&          total_supply,       //PK
         const extended_symbol&         quote_symbol,       //交易对买symbol
         const string&                  description,        //描述
         const string&                  icon_url,           //logo
         const string&                  urls,   
         const string&                  whitepaper,         //白皮书
         const string&                  issue_at,           //发行时间
         const asset&                   issue_price,        //发行价格
         const asset&                   init_price          //初始价格
      );

      using apply_action = action_wrapper<"apply"_n, &applylisting::apply>;
};
} //namespace amax
