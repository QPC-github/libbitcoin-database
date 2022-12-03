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
#ifndef LIBBITCOIN_DATABASE_SETTINGS_HPP
#define LIBBITCOIN_DATABASE_SETTINGS_HPP

#include <filesystem>
#include <bitcoin/database/define.hpp>

namespace libbitcoin {
namespace database {

/// Common database configuration settings, properties not thread safe.
class BCD_API settings
{
public:
    settings() NOEXCEPT;
    ////settings(system::chain::selection context) NOEXCEPT;

    /// Properties.
    std::filesystem::path dir;

    /// Archives.
    /// -----------------------------------------------------------------------

    uint32_t header_buckets;
    uint32_t header_size;
    uint16_t header_rate;

    uint32_t point_buckets;
    uint32_t point_size;
    uint16_t point_rate;

    uint32_t input_buckets;
    uint32_t input_size;
    uint16_t input_rate;

    uint32_t output_size;
    uint16_t output_rate;

    uint32_t puts_size;
    uint16_t puts_rate;

    uint32_t tx_buckets;
    uint32_t tx_size;
    uint16_t tx_rate;

    uint32_t txs_buckets;
    uint32_t txs_size;
    uint16_t txs_rate;

    /// Indexes.
    /// -----------------------------------------------------------------------

    uint32_t address_buckets;
    uint32_t address_size;
    uint16_t address_rate;

    uint32_t candidate_size;
    uint16_t candidate_rate;

    uint32_t confirmed_size;
    uint16_t confirmed_rate;

    uint32_t strong_bk_buckets;
    uint32_t strong_bk_size;
    uint16_t strong_bk_rate;

    uint32_t strong_tx_buckets;
    uint32_t strong_tx_size;
    uint16_t strong_tx_rate;
};

} // namespace database
} // namespace libbitcoin

#endif
