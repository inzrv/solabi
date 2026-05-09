#include "decoder.h"

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <type_traits>
#include <vector>

static_assert(solabi::AbiTag<solabi::uint_t<256>>);
static_assert(solabi::AbiTag<solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>>);
static_assert(solabi::is_static_v<solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>>);
static_assert(solabi::is_static_v<solabi::array_t<solabi::uint_t<256>, 3>>);
static_assert(!solabi::is_static_v<solabi::array_t<solabi::string_t, 2>>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::string_t>::type, std::string>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::array_t<solabi::uint_t<256>, 3>>::type,
                             std::array<intx::uint256, 3>>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::dyn_array_t<solabi::bool_t>>::type, std::vector<bool>>);

namespace
{
void write_word(bytes& data, size_t offset, size_t value)
{
    for (size_t i = 0; i < sizeof(size_t); ++i) {
        data[offset + WORD_SIZE - 1 - i] = static_cast<uint8_t>(value >> (i * 8));
    }
}
} // namespace

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

TEST(AbiDecoderSmoke, DecodesStaticFixedArray)
{
    bytes data(WORD_SIZE * 3, uint8_t{0});
    data[WORD_SIZE - 1] = 1;
    data[WORD_SIZE * 2 - 1] = 2;
    data[WORD_SIZE * 3 - 1] = 3;

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<solabi::uint_t<256>, 3>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 3);
    EXPECT_TRUE(value[0] == intx::uint256{1});
    EXPECT_TRUE(value[1] == intx::uint256{2});
    EXPECT_TRUE(value[2] == intx::uint256{3});
}

TEST(AbiDecoderSmoke, DecodesDynamicFixedArray)
{
    bytes data(WORD_SIZE * 7, uint8_t{0});

    write_word(data, 0, WORD_SIZE);
    write_word(data, WORD_SIZE, WORD_SIZE * 2);
    write_word(data, WORD_SIZE * 2, WORD_SIZE * 4);
    write_word(data, WORD_SIZE * 3, 2);
    data[WORD_SIZE * 4] = 'h';
    data[WORD_SIZE * 4 + 1] = 'i';
    write_word(data, WORD_SIZE * 5, 3);
    data[WORD_SIZE * 6] = 'b';
    data[WORD_SIZE * 6 + 1] = 'y';
    data[WORD_SIZE * 6 + 2] = 'e';

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<solabi::string_t, 2>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value[0], "hi");
    EXPECT_EQ(value[1], "bye");
}
