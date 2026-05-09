#include "decoder.h"

#include <gtest/gtest.h>

#include <type_traits>

static_assert(solabi::AbiTag<solabi::uint_t<256>>);
static_assert(solabi::AbiTag<solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>>);
static_assert(solabi::is_static_v<solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::string_t>::type, std::string>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::dyn_array_t<solabi::bool_t>>::type, std::vector<bool>>);

TEST(AbiDecoderSmoke, DecodesUintWord)
{
    bytes data(WORD_SIZE, uint8_t{0});
    data.back() = 42;

    size_t pos = 0;
    const auto value = solabi::decode<solabi::uint_t<256>>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value == intx::uint256{42});
}

TEST(AbiDecoderSmoke, DecodesBoolWord)
{
    bytes data(WORD_SIZE, uint8_t{0});
    data.back() = 1;

    size_t pos = 0;
    const auto value = solabi::decode<solabi::bool_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value);
}
