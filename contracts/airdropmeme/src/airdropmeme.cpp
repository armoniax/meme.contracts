#include <airdropmeme/airdropmeme.hpp>
#include <meme.token/meme.token.hpp>
#include <chrono>
#include <math.hpp>
#include <utils.hpp>
#include <eosio/eosio.hpp>
#include <eosio/permission.hpp>


#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + to_string((int)code) + string("]] ")  \
                                    + string("[[") + _self.to_string() + string("]] ") + msg); }
namespace meme {
void airdropmeme::setairdrop(const name& owner, const extended_asset& airdrop_quant){
   require_auth(_gstate.applynewmeme_contract);
   auto itr = _airdrop_tbl.find(airdrop_quant.quantity.symbol.raw());
   CHECKC(itr == _airdrop_tbl.end(), err::RECORD_EXISTING, "airdrop meme already found");
   _airdrop_tbl.emplace(owner, [&](auto &m) {
      m.owner           = owner;
      m.quant           = airdrop_quant;
      m.status          = "init"_n;
      m.created_at      = current_time_point();
      m.updated_at      = current_time_point();
   });
}


void airdropmeme::airdrop(const name& to, const name& bank, const asset& quantity, const string& memo){
   auto symbol = quantity.symbol;
   auto itr = _airdrop_tbl.find(symbol.raw());
   CHECKC(itr != _airdrop_tbl.end(), err::RECORD_NOT_FOUND, "airdrop meme not found");
   require_auth(itr->owner);

   CHECKC(itr->status == "init"_n, err::PARAM_ERROR, "airdrop meme not init");

   meme_token::xtoken::transfer_action act(itr->quant.contract, {_self, meme_token::xtoken::active_permission});
   act.send(_self, to, quantity, memo);

}

void airdropmeme::closeairdrop(const symbol& symbol){
   require_auth(_gstate.applynewmeme_contract);
   auto itr = _airdrop_tbl.find(symbol.raw());
   CHECKC(itr != _airdrop_tbl.end(), err::RECORD_NOT_FOUND, "airdrop meme not found");
   
   _airdrop_tbl.modify(itr, same_payer, [&](auto &m) {
      m.status = "close"_n;
   });
   
}
}