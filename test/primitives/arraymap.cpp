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
#include "../test.hpp"
#include "../storage.hpp"

BOOST_AUTO_TEST_SUITE(arraymap_tests)

template <typename Link, typename Record>
class arraymap_
  : public arraymap<Link, Record>
{
public:
    using arraymap<Link, Record>::arraymap;

    reader_ptr at_(const Link& record) const NOEXCEPT
    {
        return base::at(record);
    }

    writer_ptr push_(const Link& size=one) NOEXCEPT
    {
        return base::push(size);
    }

private:
    using base = arraymap<Link, Record>;
};

// There is no internal linkage, but we still have primary key domain.
using link5 = linkage<5>;
struct record0 { static constexpr size_t size = zero; };
struct record4 { static constexpr size_t size = 4; };
using slab_table = arraymap_<link5, record0>;
using record_table = arraymap_<link5, record4>;

// record arraymap
// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(arraymap__record_construct__empty__expected)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    const record_table instance{ body_store };
    BOOST_REQUIRE(body_file.empty());
}

BOOST_AUTO_TEST_CASE(arraymap__record_construct__non_empty__expected)
{
    constexpr auto body_size = 12345u;
    data_chunk body_file(body_size);
    test::storage body_store{ body_file };
    const record_table instance{ body_store };
    BOOST_REQUIRE_EQUAL(body_file.size(), body_size);
}

BOOST_AUTO_TEST_CASE(arraymap__record_at__terminal__false)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    const record_table instance{ body_store };
    BOOST_REQUIRE(!instance.at_(link5::terminal));
}

BOOST_AUTO_TEST_CASE(arraymap__record_at__empty__exhausted)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    const record_table instance{ body_store };
    BOOST_REQUIRE(instance.at_(0)->is_exhausted());
    BOOST_REQUIRE(instance.at_(19)->is_exhausted());
}

// slab arraymap
// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(arraymap__slab_construct__empty__expected)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    const slab_table instance{ body_store };
    BOOST_REQUIRE(body_file.empty());
}

BOOST_AUTO_TEST_CASE(arraymap__slab_construct__non_empty__expected)
{
    constexpr auto body_size = 12345u;
    data_chunk body_file(body_size);
    test::storage body_store{ body_file };
    const slab_table instance{ body_store };
    BOOST_REQUIRE_EQUAL(body_file.size(), body_size);
}

BOOST_AUTO_TEST_CASE(arraymap__slab_at__terminal__false)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    const slab_table instance{ body_store };
    BOOST_REQUIRE(!instance.at_(link5::terminal));
}

BOOST_AUTO_TEST_CASE(arraymap__slab_at__empty__exhausted)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    const slab_table instance{ body_store };
    BOOST_REQUIRE(instance.at_(0)->is_exhausted());
    BOOST_REQUIRE(instance.at_(19)->is_exhausted());
}

// push/found/at (protected interface positive tests)
// ----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(arraymap__record_readers__empty__expected)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    record_table instance{ body_store };

    auto stream0 = instance.push_();
    BOOST_REQUIRE_EQUAL(body_file.size(), record4::size);
    BOOST_REQUIRE(!stream0->is_exhausted());
    BOOST_REQUIRE(instance.at_(0));
    stream0.reset();

    auto stream1 = instance.push_();
    BOOST_REQUIRE_EQUAL(body_file.size(), 2u * record4::size);
    BOOST_REQUIRE(!stream1->is_exhausted());
    BOOST_REQUIRE(instance.at_(1));
    stream1.reset();

    // Past end is valid pointer but exhausted stream.
    BOOST_REQUIRE(instance.at_(2));
    BOOST_REQUIRE(instance.at_(2)->is_exhausted());

    // record (assumes zero fill)
    // =================================
    // 00000000 [0]
    // 00000000 [1]
}

BOOST_AUTO_TEST_CASE(arraymap__slab_readers__empty__expected)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    slab_table instance{ body_store };

    auto stream0 = instance.push_(record4::size);
    BOOST_REQUIRE_EQUAL(body_file.size(), record4::size);
    BOOST_REQUIRE(!stream0->is_exhausted());
    BOOST_REQUIRE(instance.at_(0));
    stream0.reset();

    auto stream1 = instance.push_(record4::size);
    BOOST_REQUIRE_EQUAL(body_file.size(), 2u * record4::size);
    BOOST_REQUIRE(!stream1->is_exhausted());
    BOOST_REQUIRE(instance.at_(record4::size));
    stream1.reset();

    // Past end is valid pointer but exhausted stream.
    BOOST_REQUIRE(instance.at_(2u * record4::size));
    BOOST_REQUIRE(instance.at_(2u * record4::size)->is_exhausted());

    // record (assumes zero fill)
    // =================================
    // 00000000 [0]
    // 00000000 [1]
}

// Records
// ----------------------------------------------------------------------------

// Passing link type prevents potential downcast warning (for < link5).
template <typename Link>
class test_record
{
public:
    // record bytes or zero for slab (for template).
    static constexpr size_t size = sizeof(uint32_t);

    // record count or bytes count for slab (for allocate).
    static constexpr Link count() NOEXCEPT { return 1; }

    test_record from_data(database::reader& source) NOEXCEPT
    {
        value = source.read_little_endian<uint32_t>();
        valid = source;
        return *this;
    }

    bool to_data(database::writer& sink) const NOEXCEPT
    {
        sink.write_little_endian(value);
        return sink;
    }

    uint32_t value{ 0 };
    bool valid{ false };
};

using link5 = linkage<5>;

BOOST_AUTO_TEST_CASE(arraymap__record_get__empty__invalid)
{
    data_chunk body_file;
    test::storage body_store{ body_file };
    const arraymap<link5, test_record<link5>> instance{ body_store };
    const auto record = instance.get(0);
    BOOST_REQUIRE(!record.valid);
}

BOOST_AUTO_TEST_CASE(arraymap__record_get__populated__valid)
{
    data_chunk body_file{ 0x01, 0x02, 0x03, 0x04 };
    test::storage body_store{ body_file };
    const arraymap<link5, test_record<link5>> instance{ body_store };
    const auto record = instance.get(0);
    BOOST_REQUIRE(record.valid);
    BOOST_REQUIRE_EQUAL(record.value, 0x04030201_u32);
}

BOOST_AUTO_TEST_CASE(arraymap__record_insert__get__expected)
{
    const data_chunk expected_file{ 0xd4, 0xc3, 0xb2, 0xa1 };
    data_chunk body_file;
    test::storage body_store{ body_file };
    arraymap<link5, test_record<link5>> instance{ body_store };
    BOOST_REQUIRE(instance.insert({ 0xa1b2c3d4_u32, true }));
    const auto record = instance.get(0);
    BOOST_REQUIRE(record.valid);
    BOOST_REQUIRE_EQUAL(record.value, 0xa1b2c3d4_u32);
    BOOST_REQUIRE_EQUAL(body_file, expected_file);
}

BOOST_AUTO_TEST_SUITE_END()
