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
#ifndef LIBBITCOIN_DATABASE_PRIMITIVES_SLAB_MANAGER_IPP
#define LIBBITCOIN_DATABASE_PRIMITIVES_SLAB_MANAGER_IPP

#include <mutex>
#include <shared_mutex>
#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/memory/memory.hpp>
#include <bitcoin/database/memory/storage.hpp>

namespace libbitcoin {
namespace database {
namespace primitives {

TEMPLATE
CLASS::slab_manager(Storage& file) NOEXCEPT
  : file_(file), size_(file_.logical())
{
}

TEMPLATE
Link CLASS::size() const NOEXCEPT
{
    std::shared_lock lock(mutex_);
    return file_.is_closed() ? eof : size_;
}

TEMPLATE
Link CLASS::allocate(size_t size) NOEXCEPT
{
    BC_ASSERT_MSG(size < eof, "size excess");
    const auto slab = system::possible_narrow_cast<Link>(size);

    std::unique_lock lock(mutex_);
    if (!file_.reserve(size_ + slab))
        return eof;

    const auto next = size_;
    size_ += slab;
    return next;
}

TEMPLATE
memory_ptr CLASS::get(Link position) const NOEXCEPT
{
    BC_ASSERT_MSG(link < eof, "link excess");
    const auto memory = file_.access();

    if (memory)
        memory->increment(position);

    return memory;
}

} // namespace primitives
} // namespace database
} // namespace libbitcoin

#endif
