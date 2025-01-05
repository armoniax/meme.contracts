#pragma once

#include <eosio/asset.hpp>
#include <eosio/action.hpp>

#include <string>

#include <wasm_db.hpp>

namespace meme {

using std::string;
using std::vector;
         
using namespace wasm::db;
using namespace eosio;

class airdropmeme {
public:
   
   ACTION setairdrop(const name& owner, const extended_asset& airdrop_quant);

   ACTION closeairdrop(const symbol& symbol);
   using setairdrop_action = eosio::action_wrapper<"setairdrop"_n,      &airdropmeme::setairdrop>;
   using closeairdrop_action = eosio::action_wrapper<"closeairdrop"_n,  &airdropmeme::closeairdrop>;
};
} //namespace amax
