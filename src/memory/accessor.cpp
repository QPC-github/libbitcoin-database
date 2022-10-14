/**
 * Copyright (c) 2011-2019 libbitcoin developers (see AUTHORS)
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
#include <bitcoin/database/memory/accessor.hpp>

#include <iterator>
#include <bitcoin/system.hpp>
#include <bitcoin/database/boost.hpp>
#include <bitcoin/database/define.hpp>

namespace libbitcoin {
namespace database {

BC_PUSH_WARNING(NO_THROW_IN_NOEXCEPT)

accessor::accessor(std::shared_mutex& mutex) NOEXCEPT
    : data_(nullptr), shared_lock_(mutex)
{
}

void accessor::assign(uint8_t* data) NOEXCEPT
{
    BC_ASSERT_MSG(!is_null(data), "null buffer");
    data_ = data;
}

uint8_t* accessor::buffer() NOEXCEPT
{
    return data_;
}

void accessor::increment(size_t size) NOEXCEPT
{
    BC_ASSERT_MSG(!is_null(data_), "unassigned buffer");
    BC_ASSERT(reinterpret_cast<size_t>(data_) < max_size_t - size);
    std::advance(data_, size);
}

BC_POP_WARNING()

} // namespace database
} // namespace libbitcoin
