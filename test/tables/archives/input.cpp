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
#include "../../test.hpp"
#include "../../mocks/chunk_storage.hpp"

BOOST_AUTO_TEST_SUITE(input_tests)

using namespace system;
constexpr search<table::input::slab::sk> key = base16_array("11223344556677");
const table::input::slab expected
{
    {},             // schema::input [all const static members]
    0x56341201_u32, // parent_fk
    0x00000000_u32, // index
    0x56341202_u32, // sequence
    {},             // script
    {}              // witness
};
constexpr auto slab0_size = 23u;
const data_chunk expected_file
{
    // next
    0xff, 0xff, 0xff, 0xff, 0xff,

    // key
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // slab
    0x00, 0x00, 0x00, 0x00,
    0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00,
    0x00,

    // --------------------------------------------------------------------------------------------

    // next
    0xff, 0xff, 0xff, 0xff, 0xff,

    // key
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,

    // slab
    0x01, 0x12, 0x34, 0x56,
    0x00,
    0x02, 0x12, 0x34, 0x56,
    0x00,
    0x00
};

BOOST_AUTO_TEST_CASE(input__put__get__expected)
{
    test::chunk_storage head_store{};
    test::chunk_storage body_store{};
    table::input instance{ head_store, body_store, 20 };
    BOOST_REQUIRE(instance.create());
    BOOST_REQUIRE(!instance.put_link({}, table::input::slab{}).is_terminal());
    BOOST_REQUIRE(!instance.put_link(key, expected).is_terminal());
    BOOST_REQUIRE_EQUAL(body_store.buffer(), expected_file);

    table::input::slab element{};
    BOOST_REQUIRE(instance.get(0, element));
    BOOST_REQUIRE(element == table::input::slab{});

    BOOST_REQUIRE(instance.get(slab0_size, element));
    BOOST_REQUIRE(element == expected);
}

BOOST_AUTO_TEST_CASE(input__get__only__expected)
{
    auto file = expected_file;
    test::chunk_storage head_store{};
    test::chunk_storage body_store{ file };
    table::input instance{ head_store, body_store, 20 };
    BOOST_REQUIRE_EQUAL(body_store.buffer(), expected_file);

    const auto expected_prevout = to_shared(chain::point{ one_hash, 42u });
    table::input::only element{};
    BOOST_REQUIRE(instance.get(slab0_size, element));
    BOOST_REQUIRE_EQUAL(element.sequence, expected.sequence);
    BOOST_REQUIRE(*element.script == expected.script);
    BOOST_REQUIRE(*element.witness == expected.witness);
}

BOOST_AUTO_TEST_CASE(input__get__only_from_prevout__expected)
{
    auto file = expected_file;
    test::chunk_storage head_store{};
    test::chunk_storage body_store{ file };
    table::input instance{ head_store, body_store, 20 };
    BOOST_REQUIRE_EQUAL(body_store.buffer(), expected_file);

    const auto expected_prevout = to_shared(chain::point{ one_hash, 42u });
    table::input::only_from_prevout element{ {}, expected_prevout };
    BOOST_REQUIRE(instance.get(slab0_size, element));
    BOOST_REQUIRE_EQUAL(element.input->sequence(), expected.sequence);
    BOOST_REQUIRE(element.input->script() == expected.script);
    BOOST_REQUIRE(element.input->witness() == expected.witness);
    BOOST_REQUIRE(element.input->point_ptr() == expected_prevout);
}

BOOST_AUTO_TEST_CASE(input__put__get_composite_sk__expected)
{
    test::chunk_storage head_store{};
    test::chunk_storage body_store{};
    table::input instance{ head_store, body_store, 20 };
    BOOST_REQUIRE(instance.create());
    BOOST_REQUIRE(!instance.put_link({}, table::input::slab{}).is_terminal());
    BOOST_REQUIRE(!instance.put_link(key, expected).is_terminal());
    BOOST_REQUIRE_EQUAL(body_store.buffer(), expected_file);

    table::input::slab_composite_sk element{};
    BOOST_REQUIRE(instance.get(slab0_size, element));
    BOOST_REQUIRE_EQUAL(element.key, key);
    BOOST_REQUIRE_EQUAL(element.point_fk(), 0x44332211_u32);
    BOOST_REQUIRE_EQUAL(element.point_index(), 0x00776655_u32);
}

BOOST_AUTO_TEST_CASE(input__put__get_decomposed_sk__expected)
{
    test::chunk_storage head_store{};
    test::chunk_storage body_store{};
    table::input instance{ head_store, body_store, 20 };
    BOOST_REQUIRE(instance.create());
    BOOST_REQUIRE(!instance.put_link({}, table::input::slab{}).is_terminal());
    BOOST_REQUIRE(!instance.put_link(key, expected).is_terminal());
    BOOST_REQUIRE_EQUAL(body_store.buffer(), expected_file);

    table::input::slab_decomposed_sk element{};
    BOOST_REQUIRE(instance.get(slab0_size, element));
    BOOST_REQUIRE(!element.is_null());
    BOOST_REQUIRE_EQUAL(element.point_fk, 0x44332211_u32);
    BOOST_REQUIRE_EQUAL(element.point_index, 0x00776655_u32);
}

BOOST_AUTO_TEST_CASE(input__get_decomposed_sk__null_index__expected)
{
    const table::input::slab expected_null_point
    {
        {},             // schema::input [all const static members]
        0xffffffff_u32, // parent_fk
        0xffffffff_u32, // index
        0x56341202_u32, // sequence
        {},             // script
        {}              // witness
    };
    data_chunk expected_null_point_file
    {
        // next
        0xff, 0xff, 0xff, 0xff, 0xff,

        // key [parent_fk, point_index (truncated)]
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,

        // slab
        0x00, 0x00, 0x00, 0x00,
        0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00,
        0x00
    };

    test::chunk_storage head_store{};
    test::chunk_storage body_store{ expected_null_point_file };
    table::input instance{ head_store, body_store, 20 };
    table::input::slab_decomposed_sk element{};
    BOOST_REQUIRE(instance.get(0, element));
    BOOST_REQUIRE(element.is_null());
    BOOST_REQUIRE_EQUAL(element.point_fk, 0xffffffff_u32);
    BOOST_REQUIRE_EQUAL(element.point_index, 0xffffffff_u32);
}

BOOST_AUTO_TEST_CASE(input__it__key__expected)
{
    test::chunk_storage head_store{};
    test::chunk_storage body_store{};
    table::input instance{ head_store, body_store, 20 };
    BOOST_REQUIRE(instance.create());
    BOOST_REQUIRE(!instance.put_link({}, table::input::slab{}).is_terminal());
    BOOST_REQUIRE(!instance.put_link(key, expected).is_terminal());
    BOOST_REQUIRE_EQUAL(body_store.buffer(), expected_file);

    auto it = instance.it(key);
    BOOST_REQUIRE_EQUAL(it.self(), slab0_size);
    BOOST_REQUIRE(!it.advance());
}

BOOST_AUTO_TEST_CASE(input__null_point__compose_terminals__equal)
{
    using in = table::input;
    BOOST_REQUIRE_EQUAL(in::null_point(), in::compose(in::tx::terminal, in::ix::terminal));
}

// get_parent
// slab_put_ref
// slab_decomposed_fk
// only_with_decomposed_sk

BOOST_AUTO_TEST_SUITE_END()
