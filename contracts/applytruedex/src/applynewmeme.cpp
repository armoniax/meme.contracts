#include <applytruedex/applytruedex.hpp>
#include <meme.token/meme.token.hpp>
#include <hoot.swap/hoot.swap.hpp>
#include <amax.token/amax.token.hpp>
#include <airdropmeme/airdropmeme.hpp>
#include <chrono>
#include <math.hpp>
#include <utils.hpp>
#include <eosio/eosio.hpp>
#include <eosio/permission.hpp>

#define TOKEN_TRANSFER(bank, to, quantity, memo)                                                                                           \
    {                                                                                                                                      \
        token::transfer_action act{bank, {{_self, meme_token::xtoken::active_permission}}};                                                                    \
        act.send(_self, to, quantity, memo);                                                                                               \
    }
#define CREATEHOOTSWAP(contract, user, pool1, pool2, liquidity_symbol)                                                                     \
    {                                                                                                                                      \
        amax::hootswap::create_pool_action act{contract, {{_self, meme_token::xtoken::active_permission}}};                                                          \
        act.send(user, pool1, pool2, liquidity_symbol);                                                                                    \
    }

static constexpr uint64_t RATIO_BOOST = 10000;

#define CHECKC(exp, code, msg) \
   { if (!(exp)) eosio::check(false, string("[[") + to_string((int)code) + string("]] ")  \
                                    + string("[[") + _self.to_string() + string("]] ") + msg); }

namespace meme {
using namespace std;

void applytruedex::init(   const name& admin, 
                           const name& airdrop_contract, 
                           const name& swap_contract, 
                           const name& fufi_spot_contract,
                           const name& fufi_spot_apply_contract, 
                           const name& meme_token_contract){
   require_auth( _self );

   CHECKC( is_account(admin),err::ACCOUNT_INVALID,"admin invalid:" + admin.to_string())
   _gstate.admin                 = admin; 
   _gstate.airdrop_contract      = airdrop_contract;
   _gstate.swap_contract         = swap_contract;
   _gstate.fufi_contract         = fufi_spot_contract;
   _gstate.fufi_apply_contract   = fufi_spot_apply_contract;
   _gstate.meme_token_contract   = meme_token_contract;

   _global.set(_gstate, _self);
}

void applytruedex::applymeme(const name& owner, 
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
      m.total_supply    = extended_asset{coin, _gstate.meme_token_contract};
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

void applytruedex::on_transfer(const name& from, const name& to, const asset& quantity, const string& memo){
   if(from == _self || to != _self){
      return;
   }

   auto parts        = split( memo, ":" );
   CHECKC(parts.size() == 2, err::PARAM_ERROR, "memo invalid");
   CHECKC(parts[0] == "meme", err::PARAM_ERROR, "memo invalid");
   auto from_bank    = get_first_receiver();
   auto symbol       = symbol_from_string(parts[1]);
   auto itr          = _meme_tbl.find(symbol.code().raw());
   CHECKC(itr != _meme_tbl.end(),   err::RECORD_NOT_FOUND, "meme not exists");  
   // CHECKC(itr->owner == from,       err::ACCOUNT_INVALID, "account invalid");
   eosio::print("quantity: " + itr->total_supply.quantity.to_string());
   eosio::print("precision: " + to_string(calc_precision(itr->total_supply.quantity.symbol.precision())));
   auto paid_amount  = itr->total_supply.quantity.amount/calc_precision(itr->total_supply.quantity.symbol.precision()) * itr->init_price;
   CHECKC(paid_amount == quantity.amount, err::PARAM_ERROR, "paid amount invalid"  + to_string(paid_amount) + ":" + to_string(quantity.amount));
   CHECKC(from_bank == itr->trade_symbol.get_contract(), err::PARAM_ERROR, "from bank invalid:" + from_bank.to_string()); 

   auto airdrop_amount  = itr->total_supply.quantity.amount * itr->airdrop_ratio / RATIO_BOOST;
   auto airdrop_asset   = asset(airdrop_amount, itr->total_supply.quantity.symbol);
   meme_token::xtoken::initmeme_action act(_gstate.meme_token_contract, {_self, meme_token::xtoken::active_permission});
   act.send(from, itr->total_supply.quantity, itr->airdrop_enable, itr->fee_receiver, itr->transfer_ratio, itr->destroy_ratio);
   eosio::print("initmeme end");
   extended_asset sell_ex_quant  = extended_asset{ itr->total_supply.quantity - airdrop_asset, itr->total_supply.contract};
   extended_asset buy_ex_quant   = extended_asset{quantity, from_bank};
   _create_hootswap(sell_ex_quant, buy_ex_quant);
   eosio::print("create_hootswap end");

   //set set accout perms
   meme_token::xtoken::setacctperms_action act_perm(_gstate.meme_token_contract, {_self, meme_token::xtoken::active_permission});
   std::vector<name> acccouts = {_self, _gstate.airdrop_contract, _gstate.swap_contract};
   act_perm.send(acccouts, symbol, true, true);

   
   eosio::print("setacctperms end");
   TRANSFER(_gstate.meme_token_contract, _gstate.airdrop_contract, airdrop_asset, "init:" + itr->owner.to_string());
   eosio::print("transfer end");

}

void applytruedex::_create_hootswap(const extended_asset& sell_ex_quant, const extended_asset& buy_ex_quant){
   auto from   = _self;
   auto pool1  = sell_ex_quant;
   auto pool2  = buy_ex_quant;
   if (pool1.quantity.symbol.code().to_string() > pool2.quantity.symbol.code().to_string()) {
      pool1          = buy_ex_quant;
      pool2          = sell_ex_quant;
   }

   bool is_exists = amax::hootswap::is_exists_pool(_gstate.swap_contract, pool1.get_extended_symbol(), pool2.get_extended_symbol());
   CHECKC(!is_exists, err::RECORD_EXISTING, "pool already exists");

   auto sympair   = amax::hootswap::pool_symbol(pool1.quantity.symbol, pool2.quantity.symbol);
   auto liquidity_symbol_string = add_symbol(pool1.quantity.symbol, pool2.quantity.symbol, 1);
   CREATEHOOTSWAP(_gstate.swap_contract,
             _self, pool1.get_extended_symbol(), pool2.get_extended_symbol(), 
             symbol_code(liquidity_symbol_string))

   TOKEN_TRANSFER(pool1.contract,
                  _gstate.swap_contract,
                  pool1.quantity,
                  "mint:" + sympair.to_string() + ":1:" + to_string(_rand(from, 0xFFFFFFFFFFFFFFFF)) + ":" + from.to_string())
   
   TOKEN_TRANSFER(pool2.contract,
                  _gstate.swap_contract,
                  pool2.quantity,
                  "mint:" + sympair.to_string() + ":2:" + to_string(_rand(from, 0xFFFFFFFFFFFFFFFF)) + ":" + from.to_string())
}

void applytruedex::closeairdrop(const symbol& symbol){
   auto itr = _meme_tbl.find(symbol.raw());
   CHECKC(itr != _meme_tbl.end(), err::RECORD_NOT_FOUND, "meme not found"); 
   require_auth(itr->owner);

   CHECKC(itr->airdrop_enable, err::PARAM_ERROR, "airdrop not enable");

   _meme_tbl.modify(itr, _self, [&](auto &m) {
      m.airdrop_enable = false;
   });
   
   //meme.token can transfer 
   meme_token::xtoken::closeairdrop_action act(_gstate.meme_token_contract, {_self, meme_token::xtoken::active_permission});
   act.send(symbol);
   //airdrop close airdrop 
   meme::airdropmeme::closeairdrop_action act2(_gstate.airdrop_contract, {_self, meme_token::xtoken::active_permission});
   act2.send(symbol);
}

uint64_t applytruedex::_rand(const name& user, const uint64_t& range) {
   auto        mixd      = tapos_block_prefix() * tapos_block_num() + user.value + current_time_point().sec_since_epoch();
   const char* mixedChar = reinterpret_cast<const char*>(&mixd);
   auto        result    = sha256((char*)mixedChar, sizeof(mixedChar));
   uint64_t    num1      = (uint64_t)(result.data()[0]) + (uint64_t)(result.data()[8]) * 10 + (uint64_t)(result.data()[16]) * 100 +
                  (uint64_t)(result.data()[24]) * 1000;
   uint64_t random_num = (num1 % range);
   return random_num;
}

} // namespace meme