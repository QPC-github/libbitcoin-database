/**
/// Copyright (c) 2011-2022 libbitcoin developers (see AUTHORS)
 *
/// This file is part of libbitcoin.
 *
/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Affero General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
 *
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Affero General Public License for more details.
 *
/// You should have received a copy of the GNU Affero General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_DATABASE_PRIMITIVES_HASHMAP_HPP
#define LIBBITCOIN_DATABASE_PRIMITIVES_HASHMAP_HPP

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/memory/memory.hpp>
#include <bitcoin/database/primitives/head.hpp>
#include <bitcoin/database/primitives/iterator.hpp>
#include <bitcoin/database/primitives/linkage.hpp>
#include <bitcoin/database/primitives/manager.hpp>

namespace libbitcoin {
namespace database {

/// Caution: iterator/reader/finalizer hold body remap lock until disposed.
/// These handles should be used for serialization and immediately disposed.
/// Readers and writers are always prepositioned at data, and are limited to
/// the extent the record/slab size is known (limit can always be removed).
/// Streams are always initialized from first element byte up to file limit.
template <typename Link, typename Key, size_t Size>
class hashmap
{
public:
    using iterator = database::iterator<Link, Key, Size>;

    hashmap(storage& header, storage& body, const Link& buckets) NOEXCEPT;

    /// Create from empty body/head files (not thread safe).
    bool create() NOEXCEPT;

    /// False if head or body file size incorrect (not thread safe).
    bool verify() const NOEXCEPT;

    /// Set body size into header (not thread safe).
    bool snap() NOEXCEPT;

    /// Query interface, iterator is not thread safe.
    /// -----------------------------------------------------------------------

    /// True if an instance of object with key exists.
    bool exists(const Key& key) const NOEXCEPT;

    /// Iterator holds shared lock on storage remap.
    iterator it(const Key& key) const NOEXCEPT;

    /// Allocate space for element to returned link.
    Link allocate(const Link& size) NOEXCEPT;

    /// Commit element set at link to header (becomes searchable).
    bool commit(const Key& key, const Link& link) NOEXCEPT;

    template <typename Element, if_equal<Element::size, Size> = true>
    bool set(const Link& link, const Element& element) NOEXCEPT;

    template <typename Element, if_equal<Element::size, Size> = true>
    bool get(const Link& link, Element& element) const NOEXCEPT;

    template <typename Element, if_equal<Element::size, Size> = true>
    bool get(const Key& key, Element& element) const NOEXCEPT;

    template <typename Element, if_equal<Element::size, Size> = true>
    bool put(const Key& key, const Element& element) NOEXCEPT;

    ////// In two phase commit, element and allocation sizes not coordinated.
    ////template <typename Element, if_equal<Element::size, Size> = true>
    ////bool put(const Key& key, const Element& element,
    ////    const Link& allocation) NOEXCEPT;

protected:
    template <typename Streamer>
    typename Streamer::ptr streamer(const Link& link) const NOEXCEPT;

    reader_ptr getter(const Key& key) const NOEXCEPT;
    finalizer_ptr creater(const Key& key, const Link& size) NOEXCEPT;
    finalizer_ptr committer(const Key& key, const Link& link) NOEXCEPT;

private:
    static constexpr auto is_slab = (Size == max_size_t);
    using header = database::head<Link, Key>;
    using manager = database::manager<Link, Key, Size>;

    // hash/head/push thread safe.
    header header_;

    // Thread safe.
    manager manager_;
};

template <typename Element>
using hash_map = hashmap<linkage<Element::pk>, system::data_array<Element::sk>,
    Element::size>;

} // namespace database
} // namespace libbitcoin

#define TEMPLATE template <typename Link, typename Key, size_t Size>
#define CLASS hashmap<Link, Key, Size>

#include <bitcoin/database/impl/primitives/hashmap.ipp>

#undef CLASS
#undef TEMPLATE

#endif