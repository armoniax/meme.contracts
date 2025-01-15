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

namespace fufi {
using namespace std;

void applylisting::init(   const name& admin){
   require_auth( _self );

   CHECKC( is_account(admin),err::ACCOUNT_INVALID,"admin invalid:" + admin.to_string())
   _gstate.admin                 = admin; 
   _global.set(_gstate, _self);
}

void applylisting::apply(
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
    ){
    require_auth( submiter );
    auto sell_sym = total_supply.quantity.symbol;
    auto buy_sym  = quote_coin.get_symbol();
    auto tpcode   = _get_tpcode(sell_sym, buy_sym);

    auto itr = _apply_tbl.find(tpcode.value);
    CHECKC(itr == _apply_tbl.end(), err::RECORD_EXISTING, "apply already exists");

    _apply_tbl.emplace(_self, [&](auto &m) {
        m.submiter        = submiter;
        m.tpcode          = tpcode;
        m.requester       = requester;
        m.total_supply    = total_supply;
        m.quote_coin      = quote_coin;
        m.description     = description;
        m.icon_url        = icon_url;
        m.media_urls      = media_urls;
        m.whitepaper_url  = whitepaper_url;
        m.issue_at        = issue_at;
        m.issue_price     = issue_price;
        m.status          = "init"_n;
        m.created_at      = current_time_point();
        m.updated_at      = current_time_point();
   });
}

void applylisting::on_transfer(const name& from, const name& to, const asset& quantity, const string& memo){

}

} // namespace meme