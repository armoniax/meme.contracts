#include "custodydb.hpp"
#include "idodb.hpp"

#include <wasm_db.hpp>

using namespace std;
using namespace wasm::db;
class  ido  {

  public:

    ACTION createido(const name&            owner,
                     const string&          log_url,
                     const string&          desc_en,
                     const string&          desc_cn,
                     const extended_symbol& buy_symbol,
                     const extended_symbol& sell_symbol,
                     const asset&           softcap,
                     const asset&           hardcap,
                     const asset&           ratio,
                     const int16_t&         level1_rebate_ratio,
                     const int16_t&         level2_rebate_ratio,
                     const time_point_sec&  begin_at,
                     const time_point_sec&  ended_at,
                     const int16_t&         liquidity_pool_ratio,
                     const name&            type,
                     const extended_symbol& second_buy_symbol,
                     const uint16_t&        mixing_max_ratio);

    ACTION setido(const uint64_t& ido_id, const string& log_url, const string& desc_en, const string& desc_cn);


    ACTION claimrebate(vector<uint64_t> & account_ids);
    ACTION subscribe(const name&           admin,
                     const name&           user,
                     const uint64_t&       ido_id,
                     const name&           inviter,
                     const extended_asset& first_quantity,
                     const extended_asset& second_quantity);
    ACTION delido(vector<uint64_t> & ido_ids);

    using create_ido_action     = action_wrapper<"createido"_n,     &ido::createido>;   
    using set_ido_action        = action_wrapper<"setido"_n,        &ido::setido>;
    using claim_rebate_action   = action_wrapper<"claimrebate"_n,   &ido::claimrebate>;
    using subscribe_action      = action_wrapper<"subscribe"_n,     &ido::subscribe>;
    using del_ido_action        = action_wrapper<"delido"_n,        &ido::delido>;
}; //contract redpack