#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <string>

namespace meme_token
{
    /**
     * The `meme.token` sample system contract defines the structures and actions that allow users to create, issue, and manage tokens for AMAX based blockchains. It demonstrates one way to implement a smart contract which allows for creation and management of tokens. It is possible for one to create a similar contract which suits different needs. However, it is recommended that if one only needs a token with the below listed actions, that one uses the `meme.token` contract instead of developing their own.
     *
     * The `meme.token` contract class also implements two useful public static methods: `get_supply` and `get_balance`. The first allows one to check the total supply of a specified token, created by an account and the second allows one to check the balance of a token for a specified account (the token creator account has to be specified as well).
     *
     * The `meme.token` contract manages the set of tokens, accounts and their corresponding balances, by using two internal multi-index structures: the `accounts` and `stats`. The `accounts` multi-index table holds, for each row, instances of `account` object and the `account` object holds information about the balance of one token. The `accounts` table is scoped to an eosio account, and it keeps the rows indexed based on the token's symbol.  This means that when one queries the `accounts` multi-index table for an account name the result is all the tokens that account holds at the moment.
     *
     * Similarly, the `stats` multi-index table, holds instances of `currency_stats` objects for each row, which contains information about current supply, maximum supply, and the creator account for a symbol token. The `stats` table is scoped to the token symbol.  Therefore, when one queries the `stats` table for a token symbol the result is one single entry/row corresponding to the queried symbol token if it was previously created, or nothing, otherwise.
     * The `meme.token` is base on `amax.token`, support fee of transfer
     */
    class xtoken 
    {
        public:

         static constexpr eosio::name active_permission{"active"_n};


        [[eosio::action]] void initmeme(
                    const name &issuer, const asset &maximum_supply, const bool& is_airdrop,
                    const name& fee_receiver, const uint64_t& transfer_ratio, const uint64_t& destroy_ratio,
                    const asset& airdrop_quant);

        [[eosio::action]] void closeairdrop(const symbol &symbol);


        [[eosio::action]] void retire(const asset &quantity, const string &memo);

        [[eosio::action]] void transfer(const name      &from,
                                        const name      &to,
                                        const asset     &quantity,
                                        const string    &memo);

        [[eosio::action]] void notifypayfee(const name &from, const name &to, const name& fee_receiver, const asset &fee, const string &memo);

        [[eosio::action]] void open(const name &owner, const symbol &symbol, const name &ram_payer);

        [[eosio::action]] void close(const name &owner, const symbol &symbol);
        [[eosio::action]] void feeratio(const symbol &symbol, uint64_t fee_ratio);
        [[eosio::action]] void feereceiver(const symbol &symbol, const name &fee_receiver);
        [[eosio::action]] void minfee(const symbol &symbol, const asset &min_fee_quantity);
        [[eosio::action]] void feeexempt(const symbol &symbol, const name &account, bool is_fee_exempt);
        [[eosio::action]] void pause(const symbol &symbol, bool is_paused);
        [[eosio::action]] void freezeacct(const symbol &symbol, const name &account, bool is_frozen);

        static asset get_supply(const name &token_contract_account, const symbol_code &sym_code)
        {
            stats statstable(token_contract_account, sym_code.raw());
            const auto &st = statstable.get(sym_code.raw());
            return st.supply;
        }

        static asset get_balance(const name &token_contract_account, const name &owner, const symbol_code &sym_code)
        {
            accounts accountstable(token_contract_account, owner.value);
            const auto &ac = accountstable.get(sym_code.raw());
            return ac.balance;
        }
        using initmeme_action = eosio::action_wrapper<"initmeme"_n, &xtoken::initmeme>;
        using retire_action = eosio::action_wrapper<"retire"_n, &xtoken::retire>;
        using transfer_action = eosio::action_wrapper<"transfer"_n, &xtoken::transfer>;
        using closeairdrop_action = eosio::action_wrapper<"closeairdrop"_n, &xtoken::closeairdrop>;
        using notifypayfee_action = eosio::action_wrapper<"notifypayfee"_n, &xtoken::notifypayfee>;
        using open_action = eosio::action_wrapper<"open"_n, &xtoken::open>;
        using close_action = eosio::action_wrapper<"close"_n, &xtoken::close>;
        using feeratio_action = eosio::action_wrapper<"feeratio"_n, &xtoken::feeratio>;
        using feereceiver_action = eosio::action_wrapper<"feereceiver"_n, &xtoken::feereceiver>;
        using minfee_action = eosio::action_wrapper<"minfee"_n, &xtoken::minfee>;
        using feewhitelist_action = eosio::action_wrapper<"feeexempt"_n, &xtoken::feeexempt>;
        using pause_action = eosio::action_wrapper<"pause"_n, &xtoken::pause>;
        using freezeacct_action = eosio::action_wrapper<"freezeacct"_n, &xtoken::freezeacct>;

        struct [[eosio::table]] account
        {
            asset balance;
            bool  is_frozen = false;
            bool  is_fee_exempt = false;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };

        struct [[eosio::table]] currency_stats
        {
            asset supply;
            asset max_supply;
            uint64_t total_accounts = 0;   
            name issuer;
            bool is_paused = false;
            name fee_receiver;              // fee receiver
            uint64_t fee_ratio = 0;         // fee ratio, boost 10000
            uint64_t destroy_ratio = 0;     // destroy ratio
            asset min_fee_quantity;         // min fee quantity

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
        };

        typedef eosio::multi_index<"accounts"_n, account> accounts;
        typedef eosio::multi_index<"stat"_n, currency_stats> stats;

    };

}
