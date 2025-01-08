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


        [[eosio::action]] void creatememe(
                    const name &issuer, const asset &maximum_supply, const bool& airdrop_mode,
                    const name& fee_receiver, const uint64_t& fee_ratio);

        [[eosio::action]] void retire(const asset &quantity, const string &memo);

        [[eosio::action]] void transfer(const name      &from,
                                        const name      &to,
                                        const asset     &quantity,
                                        const string    &memo);

        [[eosio::action]] void notifypayfee(const name &from, const name &to, const name& fee_receiver, const asset &fee, const string &memo);
        [[eosio::action]] void open(const name &owner, const symbol &symbol, const name &ram_payer);
        [[eosio::action]] void closeairdrop(const symbol &symbol);
        [[eosio::action]] void close(const name &owner, const symbol &symbol);
        [[eosio::action]] void feeratio(const symbol &symbol, uint64_t fee_ratio);
        [[eosio::action]] void feereceiver(const symbol &symbol, const name &fee_receiver);
        [[eosio::action]] void minfee(const symbol &symbol, const asset &min_fee_quant);
        [[eosio::action]] void feeexempt(const symbol &symbol, const name &account, bool is_fee_exempted);
        [[eosio::action]] void setacctperms( std::vector<name>& acccouts, const symbol& symbol, const bool& is_fee_exempted, const bool& airdrop_allowsend);
        static asset get_balance(const name &token_contract_account, const name &owner, const symbol_code &sym_code)
        {
            accounts accountstable(token_contract_account, owner.value);
            const auto &ac = accountstable.get(sym_code.raw());
            return ac.balance;
        }
        using creatememe_action = eosio::action_wrapper<"creatememe"_n, &xtoken::creatememe>;
        using retire_action = eosio::action_wrapper<"retire"_n, &xtoken::retire>;
        using transfer_action = eosio::action_wrapper<"transfer"_n, &xtoken::transfer>;
        using notifypayfee_action = eosio::action_wrapper<"notifypayfee"_n, &xtoken::notifypayfee>;
        using open_action = eosio::action_wrapper<"open"_n, &xtoken::open>;
        using close_action = eosio::action_wrapper<"close"_n, &xtoken::close>;
        using closeairdrop_action = eosio::action_wrapper<"closeairdrop"_n, &xtoken::closeairdrop>;
        using feeratio_action = eosio::action_wrapper<"feeratio"_n, &xtoken::feeratio>;
        using feereceiver_action = eosio::action_wrapper<"feereceiver"_n, &xtoken::feereceiver>;
        using minfee_action = eosio::action_wrapper<"minfee"_n, &xtoken::minfee>;
        using feewhitelist_action = eosio::action_wrapper<"feeexempt"_n, &xtoken::feeexempt>;
        using setacctperms_action = eosio::action_wrapper<"setacctperms"_n, &xtoken::setacctperms>;
        
        struct [[eosio::table]] account
        {
            asset balance;
            bool  is_frozen = false;
            bool  is_fee_exempted = false;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
        };
        typedef eosio::multi_index<"accounts"_n, account> accounts;

    };

}
