#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

namespace amax {


using std::string;
using std::vector;
using namespace eosio;

class hootswap  {
    public:

    TBL market_t {
        name           sympair;          // Full name liquidity symbol: pool1.pool2
        symbol_code    liquidity_symbol; // Short Name liquidity symbol: L pool1[0:3] pool2[0:3]
        extended_asset pool1;
        extended_asset pool2;
        uint16_t       fee_ratio;
        uint16_t       lp_fee_ratio;
        bool           closed = false;

        uint64_t primary_key() const { return sympair.value; }

        uint128_t get_pool() const { return (uint128_t)(pool1.quantity.symbol.raw()) << 64 || pool2.quantity.symbol.raw(); }

        market_t() {}
        market_t(const name& msympair) :
            sympair(msympair) {}

        EOSLIB_SERIALIZE(market_t, (sympair)(liquidity_symbol)(pool1)(pool2)(fee_ratio)(lp_fee_ratio)(closed))

        typedef eosio::multi_index<"markets"_n, market_t, indexed_by<"poolidx"_n, const_mem_fun<market_t, uint128_t, &market_t::get_pool>>>
        idx_t;
    };

// 1. 增加额外买卖手续费设置, 0.3%的基础上增加手续费, 手续费转到 issuer设置账号, 可修改比例和接收人, 默认0%
// 5. 交易对可增加交易白名单用户, 不触发 Anti-Whale 与 Anti-Bot, 不收取额外手续费

// 2. 增加Anti-bot开关, 开启后每 X 秒内只能交易一次, 默认关闭
// 3. 增加Anti-Whale 开关, 开启后单次买入量不能超过X, 持币量超过Y不能买入, 默认设置0不限制
// 4. 增加流动性赎回开关, 关闭后, 不能撤出流动性, 默认关闭

    struct anti_whale_s {
        bool  is_enable = false;    //是否开启
        asset max_purchase_quant;   //单次最大购买量
        asset max_holding_quant;    //最大持有量
    };

    struct tran_fee_s {     //amax-usdt
        name    receiver;       //手续费接受账号 
        int16_t buy_fee_ratio;  // 0.3% default
        int16_t sell_fee_ratio; // 0.3% default
    };

    struct anti_bot_s {
        bool           is_enable = false;    //是否开启
        int32_t        trading_interval;    //交易间隔
        time_point_sec last_tran_execute_time; //上次交易时间
    };

    [[eosio::action]] void create(name user, extended_symbol pool1, extended_symbol pool2, symbol_code liquidity_symbol);
    using create_pool_action = eosio::action_wrapper<"create"_n, &hootswap::create>;
    [[eosio::action]] void marketconf(const name &user, const name &sympair,    //amax.musdt
                                     const anti_bot_s &anti_bot,                //防机器人
                                     const anti_whale_s &anti_whale,            //防鲸
                                     const bool &is_allow_liquidity_redeem,     //是否赎回流动性
                                     const tran_fee_s &tran_fee,                //手续费
                                     const set<name> &whitelist                 // whitelist for trade
                                    );
    using marketconf_action = eosio::action_wrapper<"marketconf"_n, &hootswap::marketconf>;

    static bool is_exists_pool(name hootswappool, extended_symbol pool1, extended_symbol pool2) {
        auto            default_sympair = pool_symbol(pool1.get_symbol(), pool2.get_symbol());
        market_t::idx_t market_idx(hootswappool, hootswappool.value);
        auto            market_itr = market_idx.find(default_sympair.value);
        return market_itr != market_idx.end();
    }


    static market_t get_pool(name hootswappool, extended_symbol pool1, extended_symbol pool2) {
        auto            default_sympair = pool_symbol(pool1.get_symbol(), pool2.get_symbol());
        market_t::idx_t market_idx(hootswappool, hootswappool.value);
        auto            market_itr = market_idx.find(default_sympair.value);
        check(market_itr != market_idx.end(), "pool not found");
        return *market_itr;
    }

    static inline name pool_symbol(symbol symbol0, symbol symbol1) {
        std::string code0 = symbol0.code().to_string();
        std::string code1 = symbol1.code().to_string();
        transform(code0.begin(), code0.end(), code0.begin(), ::tolower);
        transform(code1.begin(), code1.end(), code1.begin(), ::tolower);
        std::string code = code0 + "." + code1;
        return name(code);
    }

  public:
    
};

} // namespace amax