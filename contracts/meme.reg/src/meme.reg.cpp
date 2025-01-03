#include <meme.reg/meme.reg.hpp>
#include <meme.token/meme.token.hpp>
#include <math.hpp>
#include <utils.hpp>

#define ALLOT_APPLE(farm_contract, lease_id, to, quantity, memo) \
    {   aplink::farm::allot_action(farm_contract, { {_self, active_perm} }).send( \
            lease_id, to, quantity, memo );}

namespace amax {


namespace db {

    template<typename table, typename Lambda>
    inline void set(table &tbl,  typename table::const_iterator& itr, const eosio::name& emplaced_payer,
            const eosio::name& modified_payer, Lambda&& setter )
   {
        if (itr == tbl.end()) {
            tbl.emplace(emplaced_payer, [&]( auto& p ) {
               setter(p, true);
            });
        } else {
            tbl.modify(itr, modified_payer, [&]( auto& p ) {
               setter(p, false);
            });
        }
    }

    template<typename table, typename Lambda>
    inline void set(table &tbl,  typename table::const_iterator& itr, const eosio::name& emplaced_payer,
               Lambda&& setter )
   {
      set(tbl, itr, emplaced_payer, eosio::same_payer, setter);
   }

}// namespace db


using namespace std;
using namespace amax;

#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + to_string((int)code) + string("]] ")  \
                                    + string("[[") + _self.to_string() + string("]] ") + msg); }


   void meme_reg::init( const name& admin, const name& swap_contract, const name& fufi_contract, const name& airdrop_contract){
      require_auth( _self );

      CHECKC( is_account(admin),err::ACCOUNT_INVALID,"admin invalid:" + admin.to_string())
      _gstate.admin              = admin;
      _gstate.swap_contract      = swap_contract;
      _gstate.fufi_contract      = fufi_contract;
      _gstate.airdrop_contract   = airdrop_contract;
   }

   void meme_reg::applymeme(const name& owner, 
                     const asset&      coin,
                     const string&     disc,
                     const string&     icon_url, 
                     const string&     urls,
                     const uint64_t&   airdrop_ratio,
                     const uint64_t&   destroy_ratio,      //转账手续费销毁
                     const uint64_t&   transfer_ratio,
                     const name&       fee_receiver, //转账手续费接收账户
                     const bool&       airdrop_enable,
                     const extended_symbol&  trade_symbol,
                     const uint64_t&         init_price){
      require_auth( owner );
      auto itr = _meme_tbl.find(coin.symbol.raw());
      if(itr != _meme_tbl.end()){
         CHECKC(false, err::RECORD_NOT_FOUND, "meme already exists");
      }
      
      _meme_tbl.emplace(owner, [&](auto &m) {
         m.owner           = owner;
         m.coin            = coin;
         m.disc            = disc;
         m.icon_url        = icon_url;
         m.urls            = urls;
         m.airdrop_ratio   = airdrop_ratio;
         m.destroy_ratio   = destroy_ratio;
         m.transfer_ratio  = transfer_ratio;
         m.fee_receiver    = fee_receiver;
         m.airdrop_enable  = airdrop_enable;
         m.trade_symbol    = trade_symbol;   
         m.init_price      = init_price;
         m.destroy_ratio   = destroy_ratio;
         m.status          = "init"_n;
         m.created_at      = current_time_point();
         m.updated_at      = current_time_point();
      });
}


void meme_reg::on_transfer(const name& from, const name& to, const asset& quantity, const string& memo){
   if(from == _self || to != _self){
      return;
   }
   auto from_bank =  get_first_receiver();
   auto symbol = quantity.symbol;
   auto itr = _meme_tbl.find(symbol.raw());
   CHECKC(itr == _meme_tbl.end(),   err::RECORD_NOT_FOUND, "meme not exists");  
   CHECKC(itr->owner == from,       err::ACCOUNT_INVALID, "account invalid");

   auto& meme = *itr;
   auto paid_amount = meme.coin.amount/100000000 * meme.init_price;
   CHECKC(paid_amount != quantity.amount, err::PARAM_ERROR, "paid amount invalid");
   CHECKC(from_bank == meme.trade_symbol.get_contract(), err::PARAM_ERROR, "from bank invalid"); 

   meme_token::xtoken::initmeme_action act(_gstate.swap_contract, {_self, meme_token::xtoken::active_permission});
   act.send(from, itr->coin, itr->airdrop_enable, itr->fee_receiver, itr->transfer_ratio, itr->destroy_ratio, itr->airdrop_ratio);
   



}

}//namespace amax
