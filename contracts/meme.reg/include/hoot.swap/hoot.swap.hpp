#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

namespace amax {
#define TBL struct [[eosio::table, eosio::contract("hoot.swap")]]


using std::string;
using std::vector;
using namespace eosio;

class hootswap  {

    public:
    [[eosio::action]] void idocreate(name user, extended_symbol pool1, extended_symbol pool2, symbol_code liquidity_symbol);
    using create_pool_action = eosio::action_wrapper<"idocreate"_n, &hootswap::idocreate>;

    static bool is_exists_pool(name hootswappool, extended_symbol pool1, extended_symbol pool2) {
        auto            default_sympair = pool_symbol(pool1.get_symbol(), pool2.get_symbol());
        market_t::idx_t market_idx(hootswappool, hootswappool.value);
        auto            market_itr = market_idx.find(default_sympair.value);
        return market_itr != market_idx.end();
    }

    static inline name pool_symbol(symbol symbol0, symbol symbol1) {
        std::string code0 = symbol0.code().to_string();
        std::string code1 = symbol1.code().to_string();
        transform(code0.begin(), code0.end(), code0.begin(), ::tolower);
        transform(code1.begin(), code1.end(), code1.begin(), ::tolower);
        std::string code = code0 + "." + code1;
        return name(code);
    }

  private:
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
};

} // namespace amax