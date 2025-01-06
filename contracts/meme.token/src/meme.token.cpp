#include <meme.token/meme.token.hpp>

namespace meme_token {

#ifndef ASSERT
    #define ASSERT(exp) eosio::check(exp, #exp)
#endif

#define CHECK(exp, msg) { if (!(exp)) eosio::check(false, msg); }

    template<typename Int, typename LargerInt>
    LargerInt multiply_decimal(LargerInt a, LargerInt b, LargerInt precision) {
        LargerInt tmp = a * b / precision;
        CHECK(tmp >= std::numeric_limits<Int>::min() && tmp <= std::numeric_limits<Int>::max(),
            "overflow exception of multiply_decimal");
        return tmp;
    }

    #define multiply_decimal64(a, b, precision) multiply_decimal<int64_t, int128_t>(a, b, precision)

    void xtoken::retire(const asset &quantity, const string &memo)
    {
        const auto& sym = quantity.symbol;
        auto sym_code_raw = sym.code().raw();
        check(sym.is_valid(), "invalid symbol name");
        check(memo.size() <= 256, "memo has more than 256 bytes");

        stats statstable(get_self(), sym_code_raw);
        auto existing = statstable.find(sym_code_raw);
        check(existing != statstable.end(), "token with symbol does not exist");
        const auto &st = *existing;

        require_auth(st.issuer);

        check(quantity.is_valid(), "invalid quantity");
        check(quantity.amount > 0, "must retire positive quantity");

        check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");

        auto add_count = 0;

        if(sub_balance(st, st.issuer, quantity)) {
            add_count = -1;
        }

        statstable.modify(st, same_payer, [&](auto &s)
                          { s.supply -= quantity; 
                            s.total_accounts += add_count; });

    }

    void xtoken::transfer(const name    &from,
                          const name    &to,
                          const asset   &quantity,
                          const string  &memo)
    {
        check(from != to, "cannot transfer to self");
        require_auth(from);
        check(is_account(to), "to account does not exist");
        auto sym_code_raw = quantity.symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "token of symbol does not exist");
        check(st.supply.symbol == quantity.symbol, "symbol precision mismatch");
        check(!st.is_paused, "token is paused");

        require_recipient(from);
        require_recipient(to);

        check(quantity.is_valid(), "invalid quantity");
        check(quantity.amount > 0, "must transfer positive quantity");
        check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
        check(memo.size() <= 256, "memo has more than 256 bytes");

        CHECK(quantity > st.min_fee_quantity, "quantity must larger than min fee:" + st.min_fee_quantity.to_string());
        accounts from_accts(get_self(), from.value);
        auto from_acct = from_accts.find(sym_code_raw);
        if(st.is_airdrop) {
            if(from_acct != from_accts.end()) {
                CHECK(from_acct->airdrop_allow_send, "from account is not allow send");
            }
        }

        bool fee_exempt = true;
        if(from_acct != from_accts.end()) {
            fee_exempt = from_acct->is_fee_exempt; 
        }
        accounts to_accts(get_self(), to.value);
        auto to_acct = to_accts.find(sym_code_raw);
        fee_exempt = fee_exempt | ( to_acct == to_accts.end() || !to_acct->is_fee_exempt);
    
        asset actual_recv = quantity;
        asset fee = asset(0, quantity.symbol);
        if (    st.fee_receiver.value != 0
            &&  st.fee_ratio > 0
            &&  to != st.issuer
            &&  to != st.fee_receiver )
        {
            if(!fee_exempt) {
                fee.amount = std::max( st.min_fee_quantity.amount,
                                (int64_t)multiply_decimal64(quantity.amount, st.fee_ratio, RATIO_BOOST) );
                CHECK(fee < quantity, "the calculated fee must less than quantity");
                actual_recv -= fee;
            }
        }

        auto payer = has_auth(to) ? to : from;

        auto add_count = 0;
        if(sub_balance(st, from, quantity, true)) {
            add_count = -1;
        }
        if(add_balance(st, to, actual_recv, payer, true)) {
            add_count += 1;
        }

        if (fee.amount > 0) {
            //split fee
            asset destroy_fee = asset(0, fee.symbol);
            if(st.destroy_ratio > 0) {
                destroy_fee.amount = multiply_decimal64(fee.amount, st.destroy_ratio, RATIO_BOOST);
                if(destroy_fee.amount > 0) {
                    add_balance(st, "oooo"_n, destroy_fee, payer);
                }
            }
            fee -= destroy_fee;
            if(fee.amount > 0) {
                if(add_balance(st, st.fee_receiver, fee, payer)) {
                    add_count += 1;
                }
                notifypayfee_action notifypayfee_act{ get_self(), { {get_self(), active_permission} } };
                notifypayfee_act.send( from, to, st.fee_receiver, fee, memo );
            }
        }
        statstable.modify(st, same_payer, [&](auto &s)
                          { s.total_accounts += add_count; });
    }

    /**
     * Notify pay fee.
     * Must be Triggered as inline action by transfer()
     *
     * @param from - the from account of transfer(),
     * @param to - the to account of transfer, fee payer,
     * @param fee_receiver - fee receiver,
     * @param fee - the fee of transfer to be payed,
     * @param memo - the memo of the transfer().
     * Require contract auth
     */
    void xtoken::notifypayfee(const name &from, const name &to, const name& fee_receiver, const asset &fee, const string &memo) {
        require_auth(get_self());
        require_recipient(to);
        require_recipient(fee_receiver);
    }

    bool xtoken::sub_balance(const currency_stats &st, const name &owner, const asset &value,
                             bool is_check_frozen)
    {
        accounts from_accts(get_self(), owner.value);
        const auto &from = from_accts.get(value.symbol.code().raw(), "no balance object found");
        if (is_check_frozen) {
            check(!is_account_frozen(st, owner, from), "from account is frozen");
        }
        check(from.balance.amount >= value.amount, "overdrawn balance");

      
        from_accts.modify(from, owner, [&](auto &a) {
            a.balance -= value;
        });
        if (from.balance.amount == value.amount) {
            return true;
        }
        return false;
    }

    bool xtoken::add_balance(const currency_stats &st, const name &owner, const asset &value,
                             const name &ram_payer, bool is_check_frozen)
    {
        accounts to_accts(get_self(), owner.value);
        auto to = to_accts.find(value.symbol.code().raw());
        auto ret = false;
        if (to == to_accts.end())
        {
            to_accts.emplace(ram_payer, [&](auto &a) {
                a.balance = value;
            });
            ret = true;
        }
        else
        {
            if (is_check_frozen) {
                check(!is_account_frozen(st, owner, *to), "to account is frozen");
            }
            if (to->balance.amount == 0) {
                ret = true;
            }
            to_accts.modify(to, same_payer, [&](auto &a) {
                a.balance += value;
            });
        }
        return ret;
    }

    void xtoken::open(const name &owner, const symbol &symbol, const name &ram_payer)
    {
        require_auth(ram_payer);

        check(is_account(owner), "owner account does not exist");

        auto sym_code_raw = symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "token of symbol does not exist");
        check(st.supply.symbol == symbol, "symbol precision mismatch");
        check(!st.is_paused, "token is paused");

        open_account(owner, symbol, ram_payer);
    }

    bool xtoken::open_account(const name &owner, const symbol &symbol, const name &ram_payer) {
        accounts accts(get_self(), owner.value);
        auto it = accts.find(symbol.code().raw());
        if (it == accts.end())
        {
            accts.emplace(ram_payer, [&](auto &a)
                          { a.balance = asset{0, symbol}; });
            return true;
        }
        return false;
    }

    void xtoken::close(const name &owner, const symbol &symbol)
    {
        require_auth(owner);

        auto sym_code_raw = symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "token of symbol does not exist");
        check(st.supply.symbol == symbol, "symbol precision mismatch");
        check(!st.is_paused, "token is paused");

        accounts accts(get_self(), owner.value);
        auto it = accts.find(sym_code_raw);
        check(it != accts.end(), "Balance row already deleted or never existed. Action won't have any effect.");
        check(!is_account_frozen(st, owner, *it), "account is frozen");
        check(it->balance.amount == 0, "Cannot close because the balance is not zero.");
        accts.erase(it);
    }

    void xtoken::feeratio(const symbol &symbol, uint64_t fee_ratio) {
        check(fee_ratio < RATIO_BOOST, "fee_ratio out of range");
        update_currency_field(symbol, fee_ratio, &currency_stats::fee_ratio);
    }

    void xtoken::feereceiver(const symbol &symbol, const name &fee_receiver) {
        check(is_account(fee_receiver), "Invalid account of fee_receiver");
        currency_stats st_out;
        update_currency_field(symbol, fee_receiver, &currency_stats::fee_receiver, &st_out);
        open_account(fee_receiver, symbol, st_out.issuer);
    }

    void xtoken::minfee(const symbol &symbol, const asset &min_fee_quantity) {
        check(min_fee_quantity.symbol == symbol, "symbol of min_fee_quantity  mismatch");
        check(min_fee_quantity.amount > 0, "amount of min_fee_quantity can not be negative");
        update_currency_field(symbol, min_fee_quantity, &currency_stats::min_fee_quantity);
    }

    void xtoken::feeexempt(const symbol &symbol, const name &account, bool is_fee_exempt) {
        auto sym_code_raw = symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "token of symbol does not exist");
        check(st.supply.symbol == symbol, "symbol precision mismatch");
        require_auth(st.issuer);

        accounts accts(get_self(), account.value);
        const auto &acct = accts.get(sym_code_raw, "account of token does not exist");

        accts.modify(acct, st.issuer, [&](auto &a) {
             a.is_fee_exempt = is_fee_exempt;
        });
    }

    void xtoken::pause(const symbol &symbol, bool is_paused)
    {
        update_currency_field(symbol, is_paused, &currency_stats::is_paused);
    }

    void xtoken::freezeacct(const symbol &symbol, const name &account, bool is_frozen) {
        auto sym_code_raw = symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "token of symbol does not exist");
        check(st.supply.symbol == symbol, "symbol precision mismatch");
        require_auth(st.issuer);

        accounts accts(get_self(), account.value);
        const auto &acct = accts.get(sym_code_raw, "account of token does not exist");

        accts.modify(acct, st.issuer, [&](auto &a) {
             a.is_frozen = is_frozen;
        });
    }

    template <typename Field, typename Value>
    void xtoken::update_currency_field(const symbol &symbol, const Value &v, Field currency_stats::*field,
                                       currency_stats *st_out)
    {
        auto sym_code_raw = symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "token of symbol does not exist");
        check(st.supply.symbol == symbol, "symbol precision mismatch");
        require_auth(st.issuer);
        statstable.modify(st, same_payer, [&](auto &s) {
            s.*field = v;
            if (st_out != nullptr) *st_out = s;
        });
    }


    void xtoken::setacctperms(std::vector<name>& acccouts, const symbol& symbol,  
                    const bool& is_fee_exempt, const bool& airdrop_allowsend) {
        check(has_auth( _gstate.admin) || has_auth( _gstate.applynewmeme_contract), "only admin or applynewmeme_contract can setacctperms");
        check(acccouts.size() > 0, "acccouts is empty");

        for(auto& account : acccouts) {
            check( is_account( account ), "account does not exist: " + account.to_string() );
        
            accounts acnts( get_self(), account.value );
            auto itr = acnts.find( symbol.code().raw() );
            if( itr == acnts.end() ) {
                acnts.emplace( _self, [&]( auto& a ){
                    a.balance                   = asset(0, symbol);
                    a.is_fee_exempt             = is_fee_exempt;
                    a.airdrop_allow_send        = airdrop_allowsend;
                });
            } else {
                acnts.modify( itr, _self, [&]( auto& a ) {
                    a.is_fee_exempt             = is_fee_exempt;
                    a.airdrop_allow_send        = airdrop_allowsend;
                });
            }
        }
    }
    void xtoken::initmeme(const name &issuer, const asset &maximum_supply, const bool& is_airdrop,
                    const name& fee_receiver, const uint64_t& transfer_ratio, const uint64_t& destroy_ratio,
                    const asset& airdrop_quant) {
        require_auth(_gstate.applynewmeme_contract);
        //创建token
        check(is_account(issuer), "issuer account does not exist");
        const auto &sym = maximum_supply.symbol;
        auto sym_code_raw = sym.code().raw();
        check(sym.is_valid(), "invalid symbol name");
        check(maximum_supply.is_valid(), "invalid supply");
        check(maximum_supply.amount > 0, "max-supply must be positive");

        stats statstable(get_self(), sym_code_raw);
        auto existing = statstable.find(sym_code_raw);
        check(existing == statstable.end(), "token with symbol already exists");

        statstable.emplace(get_self(), [&](auto &s) {
            s.supply            = maximum_supply;
            s.max_supply        = maximum_supply;
            s.issuer            = issuer;
            s.min_fee_quantity  = asset(0, maximum_supply.symbol);
            s.is_airdrop        = is_airdrop;
            s.fee_receiver      = fee_receiver;
            s.fee_ratio         = transfer_ratio;
            s.destroy_ratio     = destroy_ratio;
            s.total_accounts    = 1;
        });
        CHECK(airdrop_quant.symbol == maximum_supply.symbol, "airdrop_quant symbol mismatch");
        CHECK(airdrop_quant.amount > 0, "airdrop_quant must be positive");
        CHECK(airdrop_quant.amount < maximum_supply.amount, "airdrop_quant must less than maximum_supply");
        auto remain_quant = maximum_supply - airdrop_quant;
        _add_balance( _gstate.applynewmeme_contract,    maximum_supply, issuer);
    }
    void xtoken::_add_balance( const name &owner, const asset &value, const name &ram_payer)
    {
        accounts to_accts(get_self(), owner.value);
        auto to = to_accts.find(value.symbol.code().raw());
        if (to == to_accts.end()) {
            to_accts.emplace(ram_payer, [&](auto &a) {
                a.balance = value;
                a.is_fee_exempt = true;
                a.airdrop_allow_send = true;
            });
        } else { 
                to_accts.modify(to, same_payer, [&](auto &a) {
                a.balance += value;
            });
        }
    }

    void xtoken::_add_whitelist(const name &owner, const symbol &symbol, const name &ram_payer)
    {
        accounts to_accts(get_self(), owner.value);
        auto to = to_accts.find(symbol.code().raw());
        if (to == to_accts.end()) {
            to_accts.emplace(ram_payer, [&](auto &a) {
                a.balance = asset(0, symbol);
                a.is_fee_exempt = true;
                a.airdrop_allow_send = true;
            });
        } else { 
                to_accts.modify(to, same_payer, [&](auto &a) {
                a.is_fee_exempt = true;
                a.airdrop_allow_send = true;
            });
        }
    }


    void xtoken::closeairdrop( const symbol& symbol) {
        require_auth(_gstate.applynewmeme_contract);
        auto sym_code_raw = symbol.code().raw();
        stats statstable(get_self(), sym_code_raw);
        const auto &st = statstable.get(sym_code_raw, "token of symbol does not exist");
        statstable.modify(st, same_payer, [&](auto &s) {
            s.is_airdrop = false;
        });


    }
} /// namespace meme_token

