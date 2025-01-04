#include <meme.reg/meme.reg.hpp>
#include <meme.token/meme.token.hpp>
#include <hoot.swap/hoot.swap.hpp>
#include <amax.token/amax.token.hpp>
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
        hootswap::create_pool_action act{contract, {{_self, meme_token::xtoken::active_permission}}};                                                          \
        act.send(user, pool1, pool2, liquidity_symbol);                                                                                    \
    }

  static constexpr uint64_t RATIO_BOOST = 10000;


namespace meme {
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


   auto airdrop_amount = meme.coin.amount * meme.airdrop_ratio / RATIO_BOOST;
   auto airdrop_asset = asset(airdrop_amount, meme.coin.symbol);
   meme_token::xtoken::initmeme_action act(_gstate.swap_contract, {_self, meme_token::xtoken::active_permission});
   act.send(from, itr->coin, itr->airdrop_enable, itr->fee_receiver, itr->transfer_ratio, itr->destroy_ratio, airdrop_asset);

   // _create_hootswap(from, to, quantity, memo);
   
   TRANSFER(_gstate.meme_token_contract, _self, itr->coin, "meme init");
   TRANSFER(from_bank, _self, quantity, "meme init");
}

void meme_reg::_create_hootswap(const extended_asset& sell_ex_quant, const extended_asset& buy_ex_quant){
   auto from= _self;
   auto pool1 = sell_ex_quant;
   auto pool2 = buy_ex_quant;
   if (pool1.quantity.symbol.code().to_string() > pool2.quantity.symbol.code().to_string()) {
      auto pool_swap = pool1;
      pool1          = pool2;
      pool2          = pool_swap;
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

uint64_t meme_reg::_rand(const name& user, const uint64_t& range) {
    auto        mixd      = tapos_block_prefix() * tapos_block_num() + user.value + current_time_point().sec_since_epoch();
    const char* mixedChar = reinterpret_cast<const char*>(&mixd);
    auto        result    = sha256((char*)mixedChar, sizeof(mixedChar));
    uint64_t    num1      = (uint64_t)(result.data()[0]) + (uint64_t)(result.data()[8]) * 10 + (uint64_t)(result.data()[16]) * 100 +
                    (uint64_t)(result.data()[24]) * 1000;
    uint64_t random_num = (num1 % range);
    return random_num;
}


} // namespace meme