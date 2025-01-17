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
#ifndef LIBBITCOIN_DATABASE_STORE_IPP
#define LIBBITCOIN_DATABASE_STORE_IPP

#include <bitcoin/system.hpp>
#include <bitcoin/database/boost.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/file/file.hpp>

// TODO: evaluate performance benefits of concurrency.

namespace libbitcoin {
namespace database {

// Capture the result code if there is not already a code.
// This ensures all close/unload are always executed (required cleanup).
inline void first_code(code& ec, const code& result) NOEXCEPT
{
    if (!ec) ec = result;
}

TEMPLATE
CLASS::store(const settings& config) NOEXCEPT
  : configuration_(config),

    // Archive.

    header_head_(head(config.path / schema::dir::heads, schema::archive::header)),
    header_body_(body(config.path, schema::archive::header), config.header_size, config.header_rate),
    header(header_head_, header_body_, config.header_buckets),

    point_head_(head(config.path / schema::dir::heads, schema::archive::point)),
    point_body_(body(config.path, schema::archive::point), config.point_size, config.point_rate),
    point(point_head_, point_body_, config.point_buckets),

    input_head_(head(config.path / schema::dir::heads, schema::archive::input)),
    input_body_(body(config.path, schema::archive::input), config.input_size, config.input_rate),
    input(input_head_, input_body_, config.input_buckets),

    output_head_(head(config.path / schema::dir::heads, schema::archive::output)),
    output_body_(body(config.path, schema::archive::output), config.output_size, config.output_rate),
    output(output_head_, output_body_),

    puts_head_(head(config.path / schema::dir::heads, schema::archive::puts)),
    puts_body_(body(config.path, schema::archive::puts), config.puts_size, config.puts_rate),
    puts(puts_head_, puts_body_),

    tx_head_(head(config.path / schema::dir::heads, schema::archive::tx)),
    tx_body_(body(config.path, schema::archive::tx), config.tx_size, config.tx_rate),
    tx(tx_head_, tx_body_, config.tx_buckets),

    txs_head_(head(config.path / schema::dir::heads, schema::archive::txs)),
    txs_body_(body(config.path, schema::archive::txs), config.txs_size, config.txs_rate),
    txs(txs_head_, txs_body_, config.txs_buckets),

    // Indexes.

    address_head_(head(config.path / schema::dir::heads, schema::indexes::address)),
    address_body_(body(config.path, schema::indexes::address), config.address_size, config.address_rate),
    address(address_head_, address_body_, config.address_buckets),

    candidate_head_(head(config.path / schema::dir::heads, schema::indexes::candidate)),
    candidate_body_(body(config.path, schema::indexes::candidate), config.candidate_size, config.candidate_rate),
    candidate(candidate_head_, candidate_body_),

    confirmed_head_(head(config.path / schema::dir::heads, schema::indexes::confirmed)),
    confirmed_body_(body(config.path, schema::indexes::confirmed), config.confirmed_size, config.confirmed_rate),
    confirmed(confirmed_head_, confirmed_body_),

    strong_tx_head_(head(config.path / schema::dir::heads, schema::indexes::strong_tx)),
    strong_tx_body_(body(config.path, schema::indexes::strong_tx), config.strong_tx_size, config.strong_tx_rate),
    strong_tx(strong_tx_head_, strong_tx_body_, config.strong_tx_buckets),

    // Caches.

    bootstrap_head_(head(config.path / schema::dir::heads, schema::caches::bootstrap)),
    bootstrap_body_(body(config.path, schema::caches::bootstrap), config.bootstrap_size, config.bootstrap_rate),
    bootstrap(bootstrap_head_, bootstrap_body_),

    buffer_head_(head(config.path / schema::dir::heads, schema::caches::buffer)),
    buffer_body_(body(config.path, schema::caches::buffer), config.buffer_size, config.buffer_rate),
    buffer(buffer_head_, buffer_body_, config.buffer_buckets),

    neutrino_head_(head(config.path / schema::dir::heads, schema::caches::neutrino)),
    neutrino_body_(body(config.path, schema::caches::neutrino), config.neutrino_size, config.neutrino_rate),
    neutrino(neutrino_head_, neutrino_body_, config.neutrino_buckets),

    validated_bk_head_(head(config.path / schema::dir::heads, schema::caches::validated_bk)),
    validated_bk_body_(body(config.path, schema::caches::validated_bk), config.validated_bk_size, config.validated_bk_rate),
    validated_bk(validated_bk_head_, validated_bk_body_, config.validated_bk_buckets),

    validated_tx_head_(head(config.path / schema::dir::heads, schema::caches::validated_tx)),
    validated_tx_body_(body(config.path, schema::caches::validated_tx), config.validated_tx_size, config.validated_tx_rate),
    validated_tx(validated_tx_head_, validated_tx_body_, config.validated_tx_buckets),

    // Locks.
    flush_lock_(lock(config.path, schema::locks::flush)),
    process_lock_(lock(config.path, schema::locks::process))
{
}

TEMPLATE
code CLASS::create() NOEXCEPT
{
    if (!transactor_mutex_.try_lock())
        return error::transactor_lock;

    if (!process_lock_.try_lock())
    {
        transactor_mutex_.unlock();
        return error::process_lock;
    }

    if (!flush_lock_.try_lock())
    {
        /* bool */ process_lock_.try_unlock();
        transactor_mutex_.unlock();
        return error::flush_lock;
    }

    code ec{ error::success };
    static const auto heads = configuration_.path / schema::dir::heads;

    // Clear /heads, create head files, ensure existence of body files.
    if (!file::clear_directory(heads)) ec = error::clear_directory;

    else if (!file::create_file(header_head_.file())) ec = error::create_file;
    else if (!file::create_file(header_body_.file())) ec = error::create_file;
    else if (!file::create_file(point_head_.file())) ec = error::create_file;
    else if (!file::create_file(point_body_.file())) ec = error::create_file;
    else if (!file::create_file(input_head_.file())) ec = error::create_file;
    else if (!file::create_file(input_body_.file())) ec = error::create_file;
    else if (!file::create_file(output_head_.file())) ec = error::create_file;
    else if (!file::create_file(output_body_.file())) ec = error::create_file;
    else if (!file::create_file(puts_head_.file())) ec = error::create_file;
    else if (!file::create_file(puts_body_.file())) ec = error::create_file;
    else if (!file::create_file(tx_head_.file())) ec = error::create_file;
    else if (!file::create_file(tx_body_.file())) ec = error::create_file;
    else if (!file::create_file(txs_head_.file())) ec = error::create_file;
    else if (!file::create_file(txs_body_.file())) ec = error::create_file;

    else if (!file::create_file(address_head_.file())) ec = error::create_file;
    else if (!file::create_file(address_body_.file())) ec = error::create_file;
    else if (!file::create_file(candidate_head_.file())) ec = error::create_file;
    else if (!file::create_file(candidate_body_.file())) ec = error::create_file;
    else if (!file::create_file(confirmed_head_.file())) ec = error::create_file;
    else if (!file::create_file(confirmed_body_.file())) ec = error::create_file;
    else if (!file::create_file(strong_tx_head_.file())) ec = error::create_file;
    else if (!file::create_file(strong_tx_body_.file())) ec = error::create_file;

    else if (!file::create_file(bootstrap_head_.file())) ec = error::create_file;
    else if (!file::create_file(bootstrap_body_.file())) ec = error::create_file;
    else if (!file::create_file(buffer_head_.file())) ec = error::create_file;
    else if (!file::create_file(buffer_body_.file())) ec = error::create_file;
    else if (!file::create_file(neutrino_head_.file())) ec = error::create_file;
    else if (!file::create_file(neutrino_body_.file())) ec = error::create_file;
    else if (!file::create_file(validated_bk_head_.file())) ec = error::create_file;
    else if (!file::create_file(validated_bk_body_.file())) ec = error::create_file;
    else if (!file::create_file(validated_tx_head_.file())) ec = error::create_file;
    else if (!file::create_file(validated_tx_body_.file())) ec = error::create_file;

    if (!ec) ec = open_load();

    if (!ec)
    {
        // Populate /heads files and truncate body sizes to zero.
        if (!header.create()) ec = error::create_table;
        else if (!point.create()) ec = error::create_table;
        else if (!input.create()) ec = error::create_table;
        else if (!output.create()) ec = error::create_table;
        else if (!puts.create()) ec = error::create_table;
        else if (!tx.create()) ec = error::create_table;
        else if (!txs.create()) ec = error::create_table;

        else if (!address.create()) ec = error::create_table;
        else if (!candidate.create()) ec = error::create_table;
        else if (!confirmed.create()) ec = error::create_table;
        else if (!strong_tx.create()) ec = error::create_table;

        else if (!bootstrap.create()) ec = error::create_table;
        else if (!buffer.create()) ec = error::create_table;
        else if (!neutrino.create()) ec = error::create_table;
        else if (!validated_bk.create()) ec = error::create_table;
        else if (!validated_tx.create()) ec = error::create_table;
    }

    if (!ec) ec = unload_close();

    // unlock errors override ec.
    if (!flush_lock_.try_unlock()) ec = error::flush_unlock;
    if (!process_lock_.try_unlock()) ec = error::process_unlock;
    if (ec) /* bool */ file::clear_directory(configuration_.path);
    transactor_mutex_.unlock();
    return ec;
}

TEMPLATE
code CLASS::open() NOEXCEPT
{
    if (!transactor_mutex_.try_lock())
        return error::transactor_lock;

    if (!process_lock_.try_lock())
    {
        transactor_mutex_.unlock();
        return error::process_lock;
    }

    if (!flush_lock_.try_lock())
    {
        /* bool */ process_lock_.try_unlock();
        transactor_mutex_.unlock();
        return error::flush_lock;
    }

    auto ec = open_load();

    if (!ec)
    {
        if (!header.verify()) ec = error::verify_table;
        else if (!point.verify()) ec = error::verify_table;
        else if (!input.verify()) ec = error::verify_table;
        else if (!output.verify()) ec = error::verify_table;
        else if (!puts.verify()) ec = error::verify_table;
        else if (!tx.verify()) ec = error::verify_table;
        else if (!txs.verify()) ec = error::verify_table;

        else if (!address.verify()) ec = error::verify_table;
        else if (!candidate.verify()) ec = error::verify_table;
        else if (!confirmed.verify()) ec = error::verify_table;
        else if (!strong_tx.verify()) ec = error::verify_table;

        else if (!bootstrap.verify()) ec = error::verify_table;
        else if (!buffer.verify()) ec = error::verify_table;
        else if (!neutrino.verify()) ec = error::verify_table;
        else if (!validated_bk.verify()) ec = error::verify_table;
        else if (!validated_tx.verify()) ec = error::verify_table;
    }

    // This prevents close from having to follow open fail.
    if (ec)
    {
        // No need to call close() as tables were just opened.
        /* code */ unload_close();

        // unlock errors override ec.
        if (!flush_lock_.try_unlock()) ec = error::flush_unlock;
        if (!process_lock_.try_unlock()) ec = error::process_unlock;
    }

    // process and flush locks remain open until close().
    transactor_mutex_.unlock();
    return ec;
}

TEMPLATE
code CLASS::snapshot() NOEXCEPT
{
    while (!transactor_mutex_.try_lock_for(boost::chrono::seconds(1)))
    {
        // TODO: log deadlock_hint
    }

    code ec{ error::success };

    // Assumes/requires tables open/loaded.
    if (!ec) ec = header_body_.flush();
    if (!ec) ec = point_body_.flush();
    if (!ec) ec = input_body_.flush();
    if (!ec) ec = output_body_.flush();
    if (!ec) ec = puts_body_.flush();
    if (!ec) ec = tx_body_.flush();
    if (!ec) ec = txs_body_.flush();

    if (!ec) ec = address_body_.flush();
    if (!ec) ec = candidate_body_.flush();
    if (!ec) ec = confirmed_body_.flush();
    if (!ec) ec = strong_tx_body_.flush();

    if (!ec) ec = bootstrap_body_.flush();
    if (!ec) ec = buffer_body_.flush();
    if (!ec) ec = neutrino_body_.flush();
    if (!ec) ec = validated_bk_body_.flush();
    if (!ec) ec = validated_tx_body_.flush();

    if (!ec) ec = backup();
    transactor_mutex_.unlock();
    return ec;
}

TEMPLATE
code CLASS::close() NOEXCEPT
{
    if (!transactor_mutex_.try_lock())
        return error::transactor_lock;

    code ec{ error::success };

    if (!ec)
    {
        if (!header.close()) ec = error::close_table;
        else if (!point.close()) ec = error::close_table;
        else if (!input.close()) ec = error::close_table;
        else if (!output.close()) ec = error::close_table;
        else if (!puts.close()) ec = error::close_table;
        else if (!tx.close()) ec = error::close_table;
        else if (!txs.close()) ec = error::close_table;

        else if (!address.close()) ec = error::close_table;
        else if (!candidate.close()) ec = error::close_table;
        else if (!confirmed.close()) ec = error::close_table;
        else if (!strong_tx.close()) ec = error::close_table;

        else if (!bootstrap.close()) ec = error::close_table;
        else if (!buffer.close()) ec = error::close_table;
        else if (!neutrino.close()) ec = error::close_table;
        else if (!validated_bk.close()) ec = error::close_table;
        else if (!validated_tx.close()) ec = error::close_table;
    }

    first_code(ec, unload_close());

    // unlock errors override ec.
    if (!flush_lock_.try_unlock()) ec = error::flush_unlock;
    if (!process_lock_.try_unlock()) ec = error::process_unlock;
    transactor_mutex_.unlock();
    return ec;
}

TEMPLATE
const typename CLASS::transactor CLASS::get_transactor() NOEXCEPT
{
    return transactor{ transactor_mutex_ };
}

TEMPLATE
code CLASS::open_load() NOEXCEPT
{
    code ec{ error::success };

    if (!ec) ec = header_head_.open();
    if (!ec) ec = header_body_.open();
    if (!ec) ec = point_head_.open();
    if (!ec) ec = point_body_.open();
    if (!ec) ec = input_head_.open();
    if (!ec) ec = input_body_.open();
    if (!ec) ec = output_head_.open();
    if (!ec) ec = output_body_.open();
    if (!ec) ec = puts_head_.open();
    if (!ec) ec = puts_body_.open();
    if (!ec) ec = tx_head_.open();
    if (!ec) ec = tx_body_.open();
    if (!ec) ec = txs_head_.open();
    if (!ec) ec = txs_body_.open();

    if (!ec) ec = address_head_.open();
    if (!ec) ec = address_body_.open();
    if (!ec) ec = candidate_head_.open();
    if (!ec) ec = candidate_body_.open();
    if (!ec) ec = confirmed_head_.open();
    if (!ec) ec = confirmed_body_.open();
    if (!ec) ec = strong_tx_head_.open();
    if (!ec) ec = strong_tx_body_.open();

    if (!ec) ec = bootstrap_head_.open();
    if (!ec) ec = bootstrap_body_.open();
    if (!ec) ec = buffer_head_.open();
    if (!ec) ec = buffer_body_.open();
    if (!ec) ec = neutrino_head_.open();
    if (!ec) ec = neutrino_body_.open();
    if (!ec) ec = validated_bk_head_.open();
    if (!ec) ec = validated_bk_body_.open();
    if (!ec) ec = validated_tx_head_.open();
    if (!ec) ec = validated_tx_body_.open();

    if (!ec) ec = header_head_.load();
    if (!ec) ec = header_body_.load();
    if (!ec) ec = point_head_.load();
    if (!ec) ec = point_body_.load();
    if (!ec) ec = input_head_.load();
    if (!ec) ec = input_body_.load();
    if (!ec) ec = output_head_.load();
    if (!ec) ec = output_body_.load();
    if (!ec) ec = puts_head_.load();
    if (!ec) ec = puts_body_.load();
    if (!ec) ec = tx_head_.load();
    if (!ec) ec = tx_body_.load();
    if (!ec) ec = txs_head_.load();
    if (!ec) ec = txs_body_.load();

    if (!ec) ec = address_head_.load();
    if (!ec) ec = address_body_.load();
    if (!ec) ec = candidate_head_.load();
    if (!ec) ec = candidate_body_.load();
    if (!ec) ec = confirmed_head_.load();
    if (!ec) ec = confirmed_body_.load();
    if (!ec) ec = strong_tx_head_.load();
    if (!ec) ec = strong_tx_body_.load();

    if (!ec) ec = bootstrap_head_.load();
    if (!ec) ec = bootstrap_body_.load();
    if (!ec) ec = buffer_head_.load();
    if (!ec) ec = buffer_body_.load();
    if (!ec) ec = neutrino_head_.load();
    if (!ec) ec = neutrino_body_.load();
    if (!ec) ec = validated_bk_head_.load();
    if (!ec) ec = validated_bk_body_.load();
    if (!ec) ec = validated_tx_head_.load();
    if (!ec) ec = validated_tx_body_.load();

    return ec;
}

TEMPLATE
code CLASS::unload_close() NOEXCEPT
{
    code ec{ error::success };

    first_code(ec, header_head_.unload());
    first_code(ec, header_body_.unload());
    first_code(ec, point_head_.unload());
    first_code(ec, point_body_.unload());
    first_code(ec, input_head_.unload());
    first_code(ec, input_body_.unload());
    first_code(ec, output_head_.unload());
    first_code(ec, output_body_.unload());
    first_code(ec, puts_head_.unload());
    first_code(ec, puts_body_.unload());
    first_code(ec, tx_head_.unload());
    first_code(ec, tx_body_.unload());
    first_code(ec, txs_head_.unload());
    first_code(ec, txs_body_.unload());

    first_code(ec, address_head_.unload());
    first_code(ec, address_body_.unload());
    first_code(ec, candidate_head_.unload());
    first_code(ec, candidate_body_.unload());
    first_code(ec, confirmed_head_.unload());
    first_code(ec, confirmed_body_.unload());
    first_code(ec, strong_tx_head_.unload());
    first_code(ec, strong_tx_body_.unload());

    first_code(ec, bootstrap_head_.unload());
    first_code(ec, bootstrap_body_.unload());
    first_code(ec, buffer_head_.unload());
    first_code(ec, buffer_body_.unload());
    first_code(ec, neutrino_head_.unload());
    first_code(ec, neutrino_body_.unload());
    first_code(ec, validated_bk_head_.unload());
    first_code(ec, validated_bk_body_.unload());
    first_code(ec, validated_tx_head_.unload());
    first_code(ec, validated_tx_body_.unload());

    first_code(ec, header_head_.close());
    first_code(ec, header_body_.close());
    first_code(ec, point_head_.close());
    first_code(ec, point_body_.close());
    first_code(ec, input_head_.close());
    first_code(ec, input_body_.close());
    first_code(ec, output_head_.close());
    first_code(ec, output_body_.close());
    first_code(ec, puts_head_.close());
    first_code(ec, puts_body_.close());
    first_code(ec, tx_head_.close());
    first_code(ec, tx_body_.close());
    first_code(ec, txs_head_.close());
    first_code(ec, txs_body_.close());

    first_code(ec, address_head_.close());
    first_code(ec, address_body_.close());
    first_code(ec, candidate_head_.close());
    first_code(ec, candidate_body_.close());
    first_code(ec, confirmed_head_.close());
    first_code(ec, confirmed_body_.close());
    first_code(ec, strong_tx_head_.close());
    first_code(ec, strong_tx_body_.close());

    first_code(ec, bootstrap_head_.close());
    first_code(ec, bootstrap_body_.close());
    first_code(ec, buffer_head_.close());
    first_code(ec, buffer_body_.close());
    first_code(ec, neutrino_head_.close());
    first_code(ec, neutrino_body_.close());
    first_code(ec, validated_bk_head_.close());
    first_code(ec, validated_bk_body_.close());
    first_code(ec, validated_tx_head_.close());
    first_code(ec, validated_tx_body_.close());

    return ec;
}

TEMPLATE
code CLASS::backup() NOEXCEPT
{
    if (!header.backup()) return error::backup_table;
    if (!point.backup()) return error::backup_table;
    if (!input.backup()) return error::backup_table;
    if (!output.backup()) return error::backup_table;
    if (!puts.backup()) return error::backup_table;
    if (!tx.backup()) return error::backup_table;
    if (!txs.backup()) return error::backup_table;

    if (!address.backup()) return error::backup_table;
    if (!candidate.backup()) return error::backup_table;
    if (!confirmed.backup()) return error::backup_table;
    if (!strong_tx.backup()) return error::backup_table;

    if (!bootstrap.backup()) return error::backup_table;
    if (!buffer.backup()) return error::backup_table;
    if (!neutrino.backup()) return error::backup_table;
    if (!validated_bk.backup()) return error::backup_table;
    if (!validated_tx.backup()) return error::backup_table;

    static const auto primary = configuration_.path / schema::dir::primary;
    static const auto secondary = configuration_.path / schema::dir::secondary;

    if (file::is_directory(primary))
    {
        // Delete /secondary, rename /primary to /secondary.
        if (!file::clear_directory(secondary)) return error::clear_directory;
        if (!file::remove(secondary)) return error::remove_directory;
        if (!file::rename(primary, secondary)) return error::rename_directory;
    }

    // Dump /heads memory maps to /primary.
    if (!file::clear_directory(primary)) return error::create_directory;
    const auto ec = dump(primary);
    if (ec) /* bool */ file::clear_directory(primary);
    return ec;
}

// Dump memory maps of /heads to new files in /primary.
TEMPLATE
code CLASS::dump(const path& folder) NOEXCEPT
{
    auto header_buffer = header_head_.get();
    auto point_buffer = point_head_.get();
    auto input_buffer = input_head_.get();
    auto output_buffer = output_head_.get();
    auto puts_buffer = puts_head_.get();
    auto tx_buffer = tx_head_.get();
    auto txs_buffer = txs_head_.get();

    auto address_buffer = address_head_.get();
    auto candidate_buffer = candidate_head_.get();
    auto confirmed_buffer = confirmed_head_.get();
    auto strong_tx_buffer = strong_tx_head_.get();

    auto bootstrap_buffer = bootstrap_head_.get();
    auto buffer_buffer = buffer_head_.get();
    auto neutrino_buffer = neutrino_head_.get();
    auto validated_bk_buffer = validated_bk_head_.get();
    auto validated_tx_buffer = validated_tx_head_.get();

    if (!header_buffer) return error::unloaded_file;
    if (!point_buffer) return error::unloaded_file;
    if (!input_buffer) return error::unloaded_file;
    if (!output_buffer) return error::unloaded_file;
    if (!puts_buffer) return error::unloaded_file;
    if (!tx_buffer) return error::unloaded_file;
    if (!txs_buffer) return error::unloaded_file;

    if (!address_buffer) return error::unloaded_file;
    if (!candidate_buffer) return error::unloaded_file;
    if (!confirmed_buffer) return error::unloaded_file;
    if (!strong_tx_buffer) return error::unloaded_file;

    if (!bootstrap_buffer) return error::unloaded_file;
    if (!buffer_buffer) return error::unloaded_file;
    if (!neutrino_buffer) return error::unloaded_file;
    if (!validated_bk_buffer) return error::unloaded_file;
    if (!validated_tx_buffer) return error::unloaded_file;

    if (!file::create_file(head(folder, schema::archive::header),
        header_buffer->begin(), header_buffer->size()))
       return error::dump_file;

    if (!file::create_file(head(folder, schema::archive::point),
        point_buffer->begin(), point_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::archive::input),
        input_buffer->begin(), input_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::archive::output),
        output_buffer->begin(), output_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::archive::puts),
        puts_buffer->begin(), puts_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::archive::tx),
        tx_buffer->begin(), tx_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::archive::txs),
        txs_buffer->begin(), txs_buffer->size()))
        return error::dump_file;


    if (!file::create_file(head(folder, schema::indexes::address),
        address_buffer->begin(), address_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::indexes::candidate),
        candidate_buffer->begin(), candidate_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::indexes::confirmed),
        confirmed_buffer->begin(), confirmed_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::indexes::strong_tx),
        strong_tx_buffer->begin(), strong_tx_buffer->size()))
        return error::dump_file;


    if (!file::create_file(head(folder, schema::caches::bootstrap),
        bootstrap_buffer->begin(), bootstrap_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::caches::buffer),
        buffer_buffer->begin(), buffer_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::caches::neutrino),
        neutrino_buffer->begin(), neutrino_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::caches::validated_bk),
        validated_bk_buffer->begin(), validated_bk_buffer->size()))
        return error::dump_file;

    if (!file::create_file(head(folder, schema::caches::validated_tx),
        validated_tx_buffer->begin(), validated_tx_buffer->size()))
        return error::dump_file;

    return error::success;
}

TEMPLATE
code CLASS::restore() NOEXCEPT
{
    if (!transactor_mutex_.try_lock())
        return error::transactor_lock;

    if (!process_lock_.try_lock())
    {
        transactor_mutex_.unlock();
        return error::process_lock;
    }

    if (!flush_lock_.try_lock())
    {
        /* bool */ process_lock_.try_unlock();
        transactor_mutex_.unlock();
        return error::flush_lock;
    }

    code ec{ error::success };
    static const auto heads = configuration_.path / schema::dir::heads;
    static const auto primary = configuration_.path / schema::dir::primary;
    static const auto secondary = configuration_.path / schema::dir::secondary;

    if (file::is_directory(primary))
    {
        // Clear invalid /heads and recover from /primary.
        if (!file::clear_directory(heads)) ec = error::clear_directory;
        else if (!file::remove(heads)) ec = error::remove_directory;
        else if (!file::rename(primary, heads)) ec = error::rename_directory;
    }
    else if (file::is_directory(secondary))
    {
        // Clear invalid /heads and recover from /secondary.
        if (!file::clear_directory(heads)) ec = error::clear_directory;
        else if (!file::remove(heads)) ec = error::remove_directory;
        else if (!file::rename(secondary, heads)) ec = error::rename_directory;
    }
    else
    {
        ec = error::missing_backup;
    }

    if (!ec)
    {
        ec = open_load();

        if (!header.restore()) ec = error::restore_table;
        else if (!point.restore()) ec = error::restore_table;
        else if (!input.restore()) ec = error::restore_table;
        else if (!output.restore()) ec = error::restore_table;
        else if (!puts.restore()) ec = error::restore_table;
        else if (!tx.restore()) ec = error::restore_table;
        else if (!txs.restore()) ec = error::restore_table;

        else if (!address.restore()) ec = error::restore_table;
        else if (!candidate.restore()) ec = error::restore_table;
        else if (!confirmed.restore()) ec = error::restore_table;
        else if (!strong_tx.restore()) ec = error::restore_table;

        else if (!bootstrap.restore()) ec = error::restore_table;
        else if (!buffer.restore()) ec = error::restore_table;
        else if (!neutrino.restore()) ec = error::restore_table;
        else if (!validated_bk.restore()) ec = error::restore_table;
        else if (!validated_tx.restore()) ec = error::restore_table;

        else if (!ec) ec = unload_close();
    }

    // unlock errors override ec.
    if (!flush_lock_.try_unlock()) ec = error::flush_unlock;
    if (!process_lock_.try_unlock()) ec = error::process_unlock;
    transactor_mutex_.unlock();
    return ec;
}

} // namespace database
} // namespace libbitcoin

#endif
