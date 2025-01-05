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
      CHECKC(itr->owner == owner,err::PARAM_ERROR, "permission denied");
      _airdrop_tbl.emplace(owner, [&](auto &m) {
         m.owner           = owner;
         m.quant           = airdrop_quant;
         m.status          = "init"_n;
         m.created_at      = current_time_point();
         m.updated_at      = current_time_point();
      });
   }


}