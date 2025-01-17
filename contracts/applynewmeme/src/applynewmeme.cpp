#include <applynewmeme/applynewmeme.hpp>
#include <meme.token/meme.token.hpp>
#include <hoot.swap/hoot.swap.hpp>
#include <amax.token/amax.token.hpp>
#include <airdropmeme/airdropmeme.hpp>
#include <applylisting/applylisting.hpp>

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

void applynewmeme::init(   const name& admin, 
                           const name& airdrop_contract, 
                           const name& swap_contract, 
                           const name& dex_apply_contract, 
                           const name& meme_token_contract){
   require_auth( _self );

   CHECKC( is_account(admin),err::ACCOUNT_INVALID,"admin invalid:" + admin.to_string())
   _gstate.admin                 = admin; 
   _gstate.airdrop_contract      = airdrop_contract;
   _gstate.swap_contract         = swap_contract;
   _gstate.dex_apply_contract    = dex_apply_contract;
   _gstate.meme_token_contract   = meme_token_contract;

   _global.set(_gstate, _self);
}

void applynewmeme::applymeme(
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
                     ){  

   require_auth( applicant );
   auto itr = _meme_tbl.find(meme_coin.symbol.code().raw());
   if(itr != _meme_tbl.end()){
      CHECKC(false, err::RECORD_NOT_FOUND, "meme already exists");
   }

   CHECKC(airdrop_ratio >= 0 && airdrop_ratio < RATIO_BOOST, err::PARAM_ERROR, "airdrop_ratio invalid");
   CHECKC(fee_ratio >= 0 && fee_ratio < RATIO_BOOST, err::PARAM_ERROR, "fee_ratio invalid");
   CHECKC(swap_sell_fee_ratio >= 30 && swap_sell_fee_ratio < RATIO_BOOST, err::PARAM_ERROR, "swap_sell_fee_ratio invalid");
   
   _meme_tbl.emplace(applicant, [&](auto &m) {
      m.applicant             = applicant;
      m.total_supply          = extended_asset{meme_coin, _gstate.meme_token_contract};
      m.coin_name             = coin_name;
      m.quote_coin            = quote_coin; 
      m.description           = description;
      m.icon_url              = icon_url;
      m.media_urls            = media_urls;
      m.whitepaper_url        = whitepaper_url;
      m.airdrop_ratio         = airdrop_ratio;
      m.fee_ratio             = fee_ratio;
      m.swap_sell_fee_ratio   = swap_sell_fee_ratio;
      m.swap_sell_fee_receiver= swap_sell_fee_receiver;
      m.airdrop_enable        = airdrop_mode_on;
      m.status                = "init"_n;
      m.issue_at              = issue_at;
      m.created_at            = current_time_point();
   });


   auto pool1  = extended_asset{meme_coin, _gstate.meme_token_contract};
   auto pool2  = quote_coin;
   auto is_sell_coin_symbol_left = true;
   if (pool1.quantity.symbol.code().to_string() > pool2.quantity.symbol.code().to_string()) {
      pool1          = quote_coin;
      pool2          = extended_asset{meme_coin, _gstate.meme_token_contract};
      is_sell_coin_symbol_left = false;
   }

   bool is_exists = amax::hootswap::is_exists_pool(_gstate.swap_contract, pool1.get_extended_symbol(), pool2.get_extended_symbol());
   CHECKC(!is_exists, err::RECORD_EXISTING, "this symbol can't create hootswap pool");
}
//memo eg: meme:MEME
void applynewmeme::on_transfer(const name& from, const name& to, const asset& quantity, const string& memo){
   if(from == _self || to != _self){
      return;
   }

   auto parts        = split( memo, ":" );
   CHECKC(parts.size() == 2,     err::PARAM_ERROR, "memo invalid");
   CHECKC(parts[0] == "meme",    err::PARAM_ERROR, "memo invalid");
   auto from_bank    = get_first_receiver();
   auto symbol       = symbol_from_string(parts[1]);
   auto itr          = _meme_tbl.find(symbol.code().raw());
   CHECKC(itr != _meme_tbl.end(),   err::RECORD_NOT_FOUND, "meme not exists");  
   eosio::print("quantity: " + itr->total_supply.quantity.to_string() + "\n");
   eosio::print("precision: " + to_string(calc_precision(itr->total_supply.quantity.symbol.precision()))+ "\n");
   eosio::print("quantity: " + quantity.to_string() + "\n");
   eosio::print("from_bank: " + from_bank.to_string() + "\n");
   eosio::print("quote_coin: " + itr->quote_coin.quantity.to_string() + "\n");
   CHECKC(quantity == itr->quote_coin.quantity, err::PARAM_ERROR, "quantity invalid:" + quantity.to_string());
   CHECKC(from_bank == itr->quote_coin.contract, err::PARAM_ERROR, "from bank invalid:" + from_bank.to_string()); 

   auto airdrop_amount  = itr->total_supply.quantity.amount * itr->airdrop_ratio / RATIO_BOOST;
   auto airdrop_asset   = asset(airdrop_amount, itr->total_supply.quantity.symbol);

   meme_token::xtoken::creatememe_action act(_gstate.meme_token_contract, {_self, meme_token::xtoken::active_permission});
   act.send(from, itr->total_supply.quantity, itr->airdrop_enable, "oooo"_n, itr->fee_ratio);
   eosio::print("creatememe end");
   
   //set set accout perms
   meme_token::xtoken::setacctperms_action act_perm(_gstate.meme_token_contract, {_self, meme_token::xtoken::active_permission});
   std::vector<name> acccouts = {_self, _gstate.airdrop_contract, _gstate.swap_contract};
   act_perm.send(acccouts, symbol, true, true);
   
   extended_asset sell_ex_quant  = extended_asset{ itr->total_supply.quantity - airdrop_asset, itr->total_supply.contract};
   extended_asset buy_ex_quant   = extended_asset{quantity, from_bank};
   _hootswap_create(sell_ex_quant, buy_ex_quant, itr->swap_sell_fee_ratio, itr->swap_sell_fee_receiver);
   eosio::print("create_hootswap end");

   if(airdrop_asset.amount > 0){
      TRANSFER(_gstate.meme_token_contract, _gstate.airdrop_contract, airdrop_asset, "init:" + itr->applicant.to_string());
      eosio::print("transfer end");
      meme::airdropmeme::setairdrop_action act_airdrop(_gstate.airdrop_contract, {_self, meme_token::xtoken::active_permission});
      act_airdrop.send(itr->applicant, extended_asset{airdrop_asset, _gstate.meme_token_contract});
   }

}

void applynewmeme::_hootswap_create(const extended_asset& sell_ex_quant, const extended_asset& buy_ex_quant,
      const int16_t& swap_sell_fee_ratio, const name& swap_sell_fee_receiver
      ){
   auto from   = _self;
   auto pool1  = sell_ex_quant;
   auto pool2  = buy_ex_quant;
   auto is_sell_coin_symbol_left = true;
   if (pool1.quantity.symbol.code().to_string() > pool2.quantity.symbol.code().to_string()) {
      pool1          = buy_ex_quant;
      pool2          = sell_ex_quant;
      is_sell_coin_symbol_left = false;
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
   amax::hootswap::marketconf_action act_conf(_gstate.swap_contract, {_self, meme_token::xtoken::active_permission});
   amax::hootswap::anti_bot_s anti_bot = {false, 60, current_time_point()};
   amax::hootswap::anti_whale_s anti_whale = {false, asset{100000000000000, pool1.quantity.symbol}, asset{100000000000000, pool1.quantity.symbol}};
   amax::hootswap::tran_fee_s tran_fee = {swap_sell_fee_receiver, 30, swap_sell_fee_ratio};
   if(is_sell_coin_symbol_left){
      tran_fee = {swap_sell_fee_receiver, swap_sell_fee_ratio, 30};
   }
   set<name> whitelist = {};
   act_conf.send(_self, sympair, anti_bot, anti_whale, false, tran_fee, whitelist );
}  

void applynewmeme::closeairdrop(const symbol& symbol){
   auto itr = _meme_tbl.find(symbol.code().raw());
   CHECKC(itr != _meme_tbl.end(), err::RECORD_NOT_FOUND, "meme not found"); 
   require_auth(itr->applicant);

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

void applynewmeme::updatemedia(const symbol& symbol, const string& media_urls){
   auto itr = _meme_tbl.find(symbol.code().raw());
   CHECKC(itr != _meme_tbl.end(), err::RECORD_NOT_FOUND, "meme not found"); 
   require_auth(itr->applicant);
   _meme_tbl.modify(itr, _self, [&](auto &m) {
      m.media_urls = media_urls;
   });
}

uint64_t applynewmeme::_rand(const name& user, const uint64_t& range) {
   auto        mixd      = tapos_block_prefix() * tapos_block_num() + user.value + current_time_point().sec_since_epoch();
   const char* mixedChar = reinterpret_cast<const char*>(&mixd);
   auto        result    = sha256((char*)mixedChar, sizeof(mixedChar));
   uint64_t    num1      = (uint64_t)(result.data()[0]) + (uint64_t)(result.data()[8]) * 10 + (uint64_t)(result.data()[16]) * 100 +
                  (uint64_t)(result.data()[24]) * 1000;
   uint64_t random_num = (num1 % range);
   return random_num;
}

void applynewmeme::applytruedex(const symbol& symbol){
   auto itr = _meme_tbl.find(symbol.code().raw());
   CHECKC(itr != _meme_tbl.end(), err::RECORD_NOT_FOUND, "meme not found"); 

   CHECKC(itr->status == "init"_n, err::PARAM_ERROR, "meme status invalid");
   auto quote_symbol    = itr->quote_coin.quantity.symbol;
   auto market_limit    = _gstate.mcap_list_threshold[quote_symbol];
   uint64_t current_price = 0;
   auto market_value    = _get_current_market_value(itr->quote_coin.get_extended_symbol(), itr->total_supply.get_extended_symbol(), current_price);
   CHECKC(market_value >= market_limit, err::PARAM_ERROR, "market value invalid");
   auto issue_price     = itr->quote_coin.quantity.amount / (itr->total_supply.quantity.amount / 100000000);
   tyche::applylisting::apply_action act(_gstate.dex_apply_contract, {_self, meme_token::xtoken::active_permission});
   
   act.send(_self, itr->applicant, itr->total_supply, itr->quote_coin.get_extended_symbol(),
            itr->description, itr->icon_url, itr->media_urls, 
            itr->whitepaper_url, itr->issue_at, 
            issue_price,  current_price);

}

asset applynewmeme::_get_current_market_value(
      const extended_symbol& buy_symbol, 
      const extended_symbol& sell_symbol, 
      uint64_t& current_price){
   auto from   = _self;
   auto pool1  = sell_symbol;
   auto pool2  = buy_symbol;
   if (pool1.get_symbol().code().to_string() > pool2.get_symbol().code().to_string()) {
      pool1          = buy_symbol;
      pool2          = sell_symbol;
   }

   auto market = amax::hootswap::get_pool(_gstate.swap_contract, pool1, pool2);
   auto market_value = market.pool1.quantity;
   if(market.pool2.get_extended_symbol() == buy_symbol){
      market_value   = market.pool2.quantity;
      current_price  = market.pool2.quantity.amount / (market.pool1.quantity.amount /100000000);
   } else {
      market_value   = market.pool1.quantity;
      current_price  = market.pool1.quantity.amount / (market.pool2.quantity.amount /100000000);
   }
   return market_value;
}


void applynewmeme::addmcap(const symbol& symbol, const asset& threshold){
   require_auth( _self );
   _gstate.mcap_list_threshold[symbol] = threshold;
   _global.set(_gstate, _self);

}

void applynewmeme::clearmeme(const symbol& symbol){
   require_auth( _self );
   auto itr = _meme_tbl.find(symbol.code().raw());
   if(itr != _meme_tbl.end()){
      _meme_tbl.erase(itr);
   }
}


} // namespace meme