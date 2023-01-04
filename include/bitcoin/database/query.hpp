/**
 * Copyright (c) 2011-2022 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_DATABASE_QUERY_HPP
#define LIBBITCOIN_DATABASE_QUERY_HPP

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/primitives/primitives.hpp>
#include <bitcoin/database/tables/context.hpp>
#include <bitcoin/database/tables/tables.hpp>

namespace libbitcoin {
namespace database {

/// Database type aliases.
using point_link = table::point::link;
using input_link = table::input::link;
using output_link = table::output::link;
using tx_link = table::transaction::link;
using height_link = table::height::link;
using header_link = table::header::link;
using tx_links = std_vector<tx_link::integer>;
using input_links = std_vector<input_link::integer>;
using output_links = std_vector<output_link::integer>;
using header_links = std_vector<header_link::integer>;

template <typename Store>
class query
{
public:
    /// Query type aliases.
    using block = system::chain::block;
    using point = system::chain::point;
    using input = system::chain::input;
    using output = system::chain::output;
    using header = system::chain::header;
    using transaction = system::chain::transaction;
    using inputs_ptr = system::chain::inputs_ptr;
    using outputs_ptr = system::chain::outputs_ptr;
    using transactions_ptr = system::chain::transactions_ptr;
    using heights = std_vector<size_t>;
    using filter = system::data_chunk;

    query(Store& value) NOEXCEPT;

    /// Initialization (natural-keyed).
    /// -----------------------------------------------------------------------

    inline bool is_initialized() NOEXCEPT;
    inline size_t get_top_confirmed() NOEXCEPT;
    inline size_t get_top_candidate() NOEXCEPT;
    size_t get_fork() NOEXCEPT;
    size_t get_last_associated_from(size_t height) NOEXCEPT;
    hashes get_all_unassociated_above(size_t height) NOEXCEPT;
    hashes get_locator(const heights& heights) NOEXCEPT;

    /// Translation.
    /// -----------------------------------------------------------------------

    /// search key (entry)
    inline header_link to_candidate(size_t height) NOEXCEPT;
    inline header_link to_confirmed(size_t height) NOEXCEPT;
    inline header_link to_header(const hash_digest& key) NOEXCEPT;
    inline point_link to_point(const hash_digest& key) NOEXCEPT;
    inline tx_link to_tx(const hash_digest& key) NOEXCEPT;
    ////input_links to_spenders(const point& prevout) NOEXCEPT;

    /// put to tx (reverse navigation)
    /// Terminal implies fault if link verified.
    tx_link to_input_tx(const input_link& link) NOEXCEPT;
    tx_link to_output_tx(const output_link& link) NOEXCEPT;
    tx_link to_prevout_tx(const input_link& link) NOEXCEPT;

    /// point to put (forward navigation)
    /// Terminal implies fault if link/index verified.
    input_link to_input(const tx_link& link, uint32_t input_index) NOEXCEPT;
    output_link to_output(const tx_link& link, uint32_t output_index) NOEXCEPT;
    output_link to_prevout(const input_link& link) NOEXCEPT;

    /// tx to blocks (reverse navigation)
    /// There is no reverse index for unconfirmed tx->block.
    /// Terminal implies not strong.
    header_link to_strong_by(const tx_link& link) NOEXCEPT;

    /// output to spenders (reverse navigation)
    /// Empty implies no spenders.
    input_links to_spenders(const output_link& link) NOEXCEPT;
    input_links to_spenders(const point& prevout) NOEXCEPT;
    input_links to_spenders(const tx_link& link,
        uint32_t output_index) NOEXCEPT;

    /// tx to puts (forward navigation)
    /// Empty implies fault if link verified.
    input_links to_tx_inputs(const tx_link& link) NOEXCEPT;
    output_links to_tx_outputs(const tx_link& link) NOEXCEPT;

    /// block to puts/txs (forward navigation)
    /// Empty implies fault if link associated.
    input_links to_block_inputs(const header_link& link) NOEXCEPT;
    output_links to_block_outputs(const header_link& link) NOEXCEPT;
    tx_links to_transactions(const header_link& link) NOEXCEPT;

    /// Archival (natural-keyed).
    /// -----------------------------------------------------------------------

    /// False implies fault if associated.
    inline bool is_associated(const header_link& link) NOEXCEPT;
    inline bool is_header(const hash_digest& key) NOEXCEPT;
    inline bool is_block(const hash_digest& key) NOEXCEPT;
    inline bool is_tx(const hash_digest& key) NOEXCEPT;

    /// False implies fault.
    inline bool set(const header& header, const context& ctx) NOEXCEPT;
    inline bool set(const block& block, const context& ctx) NOEXCEPT;
    inline bool set(const transaction& tx) NOEXCEPT;

    /// False implies not fully populated.
    bool populate(const input& input) NOEXCEPT;
    bool populate(const transaction& tx) NOEXCEPT;
    bool populate(const block& block) NOEXCEPT;

    /// Archival (foreign-keyed).
    /// -----------------------------------------------------------------------

    /// Empty implies fault if link verified/associated, null_hash is fault.
    hashes get_txs(const header_link& link) NOEXCEPT;

    /// Null implies fault if link verified/associated.
    inputs_ptr get_inputs(const tx_link& link) NOEXCEPT;
    outputs_ptr get_outputs(const tx_link& link) NOEXCEPT;
    transactions_ptr get_transactions(const header_link& link) NOEXCEPT;

    /// Null implies fault if link verified/associated.
    header::cptr get_header(const header_link& link) NOEXCEPT;
    block::cptr get_block(const header_link& link) NOEXCEPT;
    transaction::cptr get_tx(const tx_link& link) NOEXCEPT;
    output::cptr get_output(const output_link& link) NOEXCEPT;
    input::cptr get_input(const input_link& link) NOEXCEPT;
    point::cptr get_point(const input_link& link) NOEXCEPT;
    inputs_ptr get_spenders(const output_link& link) NOEXCEPT;

    /// Null implies fault if link verified/associated.
    output::cptr get_output(const point& prevout) NOEXCEPT;
    output::cptr get_output(const tx_link& link, uint32_t output_index) NOEXCEPT;
    input::cptr get_input(const tx_link& link, uint32_t input_index) NOEXCEPT;
    inputs_ptr get_spenders(const tx_link& link, uint32_t output_index) NOEXCEPT;

    /// Terminal implies fault.
    header_link set_link(const header& header, const context& ctx) NOEXCEPT;
    header_link set_link(const block& block, const context& ctx) NOEXCEPT;
    tx_link set_link(const transaction& tx) NOEXCEPT;

    /// False implies fault.
    bool set(const header_link& link, const hashes& hashes) NOEXCEPT;
    bool set(const header_link& link, const tx_links& links) NOEXCEPT;

    /// Validation (foreign-keyed).
    /// -----------------------------------------------------------------------

    /// Terminal implies fault if link verified/associated.
    height_link get_header_height(const header_link& link) NOEXCEPT;

    /// error::unknown implies fault.
    code get_block_state(const header_link& link) NOEXCEPT;
    code get_block_state(uint64_t& fees, const header_link& link) NOEXCEPT;
    code get_tx_state(const tx_link& link, const context& ctx) NOEXCEPT;
    code get_tx_state(uint64_t& fee, size_t& sigops, const tx_link& link,
        const context& ctx) NOEXCEPT;

    /// False implies fault.
    bool set_block_preconfirmable(const header_link& link) NOEXCEPT;
    bool set_block_unconfirmable(const header_link& link) NOEXCEPT;
    bool set_block_confirmable(const header_link& link, uint64_t fees) NOEXCEPT;

    /// False implies fault.
    bool set_tx_preconnected(const tx_link& link, const context& ctx) NOEXCEPT;
    bool set_tx_disconnected(const tx_link& link, const context& ctx) NOEXCEPT;
    bool set_tx_connected(const tx_link& link, const context& ctx,
        uint64_t fee, size_t sigops) NOEXCEPT;

    /// Confirmation (foreign-keyed).
    /// -----------------------------------------------------------------------

    /// These verify strong against confirmation height.
    bool is_candidate_block(const header_link& link) NOEXCEPT;
    bool is_confirmed_block(const header_link& link) NOEXCEPT;
    bool is_confirmed_tx(const tx_link& link) NOEXCEPT;
    bool is_confirmed_input(const input_link& link) NOEXCEPT;
    bool is_confirmed_output(const output_link& link) NOEXCEPT;

    /// These rely on strong (use only for confirmation process).
    /// Set strong during confirmation process or current block will be missed.
    /// Spent implies confirmed other spenders of output that input spends.
    /// Confirmability assumes validy (prevouts exists, scripts validated).
    bool is_spent(const input_link& link) NOEXCEPT;
    bool is_mature(const input_link& link, size_t height) NOEXCEPT;
    bool is_confirmable_block(const header_link& link, size_t height) NOEXCEPT;

    /// False implies fault if link associated.
    /// Tx->block association relies on strong (confirmed or pending).
    bool set_strong(const header_link& link) NOEXCEPT;
    bool set_unstrong(const header_link& link) NOEXCEPT;

    /// False implies fault.
    bool initialize(const block& genesis) NOEXCEPT;
    bool push_confirmed(const header_link& link) NOEXCEPT;
    bool push_candidate(const header_link& link) NOEXCEPT;
    bool pop_confirmed() NOEXCEPT;
    bool pop_candidate() NOEXCEPT;

    /// Optional Tables.
    /// -----------------------------------------------------------------------

    /// Address (natural-keyed).
    /// Terminal implies not found, false implies fault.
    hash_digest address_hash(const output& output) NOEXCEPT;
    output_link get_address(const hash_digest& key) NOEXCEPT;
    bool set_address(const hash_digest& key, const output_link& link) NOEXCEPT;
    bool set_address(const output& output) NOEXCEPT;

    /// Neutrino (foreign-keyed).
    /// Empty/null_hash implies not found, false implies fault.
    filter get_filter(const header_link& link) NOEXCEPT;
    hash_digest get_filter_head(const header_link& link) NOEXCEPT;
    bool set_filter(const header_link& link, const hash_digest& head,
        const filter& body) NOEXCEPT;

    /// Buffer (foreign-keyed).
    /// Null implies not found, false implies fault.
    transaction::cptr get_buffered_tx(const tx_link& link) NOEXCEPT;
    bool set_buffered_tx(const tx_link& link, const transaction& tx) NOEXCEPT;

    /// Bootstrap (natural-keyed).
    /// Empty implies empty table, false implies height exceeds confirmed top.
    hashes get_bootstrap() NOEXCEPT;
    bool set_bootstrap(size_t height) NOEXCEPT;

protected:
    using input_key = table::input::search_key;
    using puts_link = table::puts::link;
    using txs_link = table::txs::link;

    /// Empty implies fault if link associated.
    inline txs_link to_txs(const header_link& link) NOEXCEPT;
    input_links to_spenders(const table::input::search_key& key) NOEXCEPT;
    bool is_mature_point(const point_link& link, size_t height) NOEXCEPT;
    bool is_spent_point(const input_link& self,
        const table::input::search_key& key) NOEXCEPT;

    inline input_key make_foreign_point(const point& prevout) NOEXCEPT;
    inline code to_block_code(linkage<schema::code>::integer value) NOEXCEPT;
    inline code to_tx_code(linkage<schema::code>::integer value) NOEXCEPT;
    inline bool is_sufficient(const context& current,
        const context& evaluated) NOEXCEPT;

private:
    Store& store_;
};

} // namespace database
} // namespace libbitcoin

#define TEMPLATE template <typename Store>
#define CLASS query<Store>

#include <bitcoin/database/impl/query.ipp>

#undef CLASS
#undef TEMPLATE

#endif
