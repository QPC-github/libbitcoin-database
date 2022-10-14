///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2021 libbitcoin-database developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef LIBBITCOIN_DATABASE_HPP
#define LIBBITCOIN_DATABASE_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <bitcoin/system.hpp>
#include <bitcoin/database/boost.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/settings.hpp>
#include <bitcoin/database/version.hpp>
#include <bitcoin/database/locks/conditional_lock.hpp>
#include <bitcoin/database/locks/file_lock.hpp>
#include <bitcoin/database/locks/flush_lock.hpp>
#include <bitcoin/database/locks/interprocess_lock.hpp>
#include <bitcoin/database/locks/locks.hpp>
#include <bitcoin/database/locks/scope_lock.hpp>
#include <bitcoin/database/locks/sequential_lock.hpp>
#include <bitcoin/database/memory/accessor.hpp>
#include <bitcoin/database/memory/file.hpp>
#include <bitcoin/database/memory/file_storage.hpp>
#include <bitcoin/database/memory/memory.hpp>
#include <bitcoin/database/memory/storage.hpp>
////#include <bitcoin/database/primitives/iterable.hpp>
////#include <bitcoin/database/primitives/iterator.hpp>
////#include <bitcoin/database/primitives/primitives.hpp>
////#include <bitcoin/database/primitives/record_manager.hpp>
////#include <bitcoin/database/primitives/slab_manager.hpp>
////#include <bitcoin/database/primitives/elements/element.hpp>
////#include <bitcoin/database/primitives/elements/keyed_record.hpp>
////#include <bitcoin/database/primitives/elements/keyed_slab.hpp>
////#include <bitcoin/database/primitives/elements/record.hpp>
////#include <bitcoin/database/primitives/elements/slab.hpp>
#include <bitcoin/database/primitives_/hash_table.hpp>
#include <bitcoin/database/primitives_/hash_table_header.hpp>
#include <bitcoin/database/primitives_/hash_table_multimap.hpp>
#include <bitcoin/database/primitives_/list.hpp>
#include <bitcoin/database/primitives_/list_element.hpp>
#include <bitcoin/database/primitives_/list_iterator.hpp>
#include <bitcoin/database/primitives_/primitives.hpp>
#include <bitcoin/database/primitives_/record_manager.hpp>
#include <bitcoin/database/primitives_/slab_manager.hpp>

#endif
