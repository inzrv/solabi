#include <solabi/decoder.h>
#include <solabi/utils.h>
#include "abi_vectors.h"
#include "test_types.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <tuple>

namespace
{
using solabi::from_hex;
} // namespace

TEST(AbiDecoderComposites, RejectsDynamicArrayWithOffsetOverflow)
{
    const auto data = from_hex("000000000000000000000000000000000000000000000000fffffffffffffff0");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::dyn_array_t<solabi::uint_t<256>>>(
                     bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderComposites, DecodesStaticFixedArray)
{
    const auto data = from_hex(abi_test_vectors::kUint256Array3);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<solabi::uint_t<256>, 3>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 3);
    EXPECT_TRUE(value[0] == intx::uint256{1});
    EXPECT_TRUE(value[1] == intx::uint256{2});
    EXPECT_TRUE(value[2] == intx::uint256{3});
}

TEST(AbiDecoderComposites, DecodesDynamicFixedArray)
{
    const auto data = from_hex(abi_test_vectors::kStringArray2);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<solabi::string_t, 2>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value[0], "hi");
    EXPECT_EQ(value[1], "bye");
}

TEST(AbiDecoderComposites, DecodesDynamicArray)
{
    const auto data = from_hex(abi_test_vectors::kUint256DynArray);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::dyn_array_t<solabi::uint_t<256>>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    ASSERT_EQ(value.size(), 3);
    EXPECT_TRUE(value[0] == intx::uint256{1});
    EXPECT_TRUE(value[1] == intx::uint256{2});
    EXPECT_TRUE(value[2] == intx::uint256{3});
}

TEST(AbiDecoderComposites, DecodesDynamicTuple)
{
    const auto data = from_hex(abi_test_vectors::kTupleUint256String);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::tuple_t<solabi::uint_t<256>, solabi::string_t>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(std::get<0>(value) == intx::uint256{7});
    EXPECT_EQ(std::get<1>(value), "hi");
}

TEST(AbiDecoderComposites, DecodesStaticStruct)
{
    const auto data = from_hex(abi_test_vectors::kStaticRecord);

    size_t pos = 0;
    const auto value = solabi::decode<StaticRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 2);
    EXPECT_TRUE(value.id == intx::uint256{9});
    EXPECT_TRUE(value.active);
}

TEST(AbiDecoderComposites, DecodesDynamicStructAsAbiTag)
{
    const auto data = from_hex(abi_test_vectors::kTupleUint256String);

    size_t pos = 0;
    const auto value = solabi::decode<DynamicRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value.id == intx::uint256{7});
    EXPECT_EQ(value.label, "hi");
}

TEST(AbiDecoderComposites, DecodesUserDefinedTypeWithConstructor)
{
    const auto data = from_hex(abi_test_vectors::kTupleUint256String);

    size_t pos = 0;
    const auto value = solabi::decode<ConstructorRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value.id == intx::uint256{7});
    EXPECT_EQ(value.label, "hi");
}

TEST(AbiDecoderComposites, DecodesStringDynamicArray)
{
    const auto data = from_hex(abi_test_vectors::kStringDynArray);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::dyn_array_t<solabi::string_t>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    ASSERT_EQ(value.size(), 2);
    EXPECT_EQ(value[0], "hi");
    EXPECT_EQ(value[1], "bye");
}

TEST(AbiDecoderComposites, DecodesStaticStructFixedArray)
{
    const auto data = from_hex(abi_test_vectors::kStaticRecordArray2);

    size_t pos = 0;
    const auto value =
        solabi::decode<solabi::array_t<StaticRecord, 2>>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 4);
    EXPECT_TRUE(value[0].id == intx::uint256{1});
    EXPECT_TRUE(value[0].active);
    EXPECT_TRUE(value[1].id == intx::uint256{2});
    EXPECT_FALSE(value[1].active);
}

TEST(AbiDecoderComposites, DecodesDynamicStructFixedArray)
{
    const auto data = from_hex(abi_test_vectors::kDynamicRecordFixedArray2);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<DynamicRecord, 2>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value[0].id == intx::uint256{1});
    EXPECT_EQ(value[0].label, "one");
    EXPECT_TRUE(value[1].id == intx::uint256{2});
    EXPECT_EQ(value[1].label, "two");
}

TEST(AbiDecoderComposites, DecodesDynamicStructDynamicArray)
{
    const auto data = from_hex(abi_test_vectors::kDynamicRecordArray2);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::dyn_array_t<DynamicRecord>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    ASSERT_EQ(value.size(), 2);
    EXPECT_TRUE(value[0].id == intx::uint256{1});
    EXPECT_EQ(value[0].label, "one");
    EXPECT_TRUE(value[1].id == intx::uint256{2});
    EXPECT_EQ(value[1].label, "two");
}

TEST(AbiDecoderComposites, DecodesDynamicArrayOfFixedArrays)
{
    const auto data = from_hex(abi_test_vectors::kUint256PairDynArray);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::dyn_array_t<solabi::array_t<solabi::uint_t<256>, 2>>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    ASSERT_EQ(value.size(), 2);
    EXPECT_TRUE(value[0][0] == intx::uint256{1});
    EXPECT_TRUE(value[0][1] == intx::uint256{2});
    EXPECT_TRUE(value[1][0] == intx::uint256{3});
    EXPECT_TRUE(value[1][1] == intx::uint256{4});
}

TEST(AbiDecoderComposites, DecodesFixedArrayOfDynamicArrays)
{
    const auto data = from_hex(abi_test_vectors::kUint256DynArrayFixedArray2);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<solabi::dyn_array_t<solabi::uint_t<256>>, 2>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    ASSERT_EQ(value[0].size(), 2);
    ASSERT_EQ(value[1].size(), 1);
    EXPECT_TRUE(value[0][0] == intx::uint256{1});
    EXPECT_TRUE(value[0][1] == intx::uint256{2});
    EXPECT_TRUE(value[1][0] == intx::uint256{3});
}

TEST(AbiDecoderComposites, DecodesNestedStaticStruct)
{
    const auto data = from_hex(abi_test_vectors::kNestedStaticRecord);

    size_t pos = 0;
    const auto value =
        solabi::decode<NestedStaticRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 3);
    EXPECT_TRUE(value.owner.id == intx::uint256{9});
    EXPECT_TRUE(value.owner.active);
    EXPECT_TRUE(value.score == intx::uint256{11});
}

TEST(AbiDecoderComposites, DecodesNestedDynamicStruct)
{
    const auto data = from_hex(abi_test_vectors::kNestedDynamicRecord);

    size_t pos = 0;
    const auto value =
        solabi::decode<NestedDynamicRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value.owner.id == intx::uint256{9});
    EXPECT_TRUE(value.owner.active);
    EXPECT_TRUE(value.payload.id == intx::uint256{7});
    EXPECT_EQ(value.payload.label, "inner");
    EXPECT_EQ(value.note, "outer");
}

TEST(AbiDecoderComposites, DecodesDeepNestedStruct)
{
    const auto data = from_hex(abi_test_vectors::kDeepRecord);

    size_t pos = 0;
    const auto value = solabi::decode<DeepRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value.nested.owner.id == intx::uint256{9});
    EXPECT_TRUE(value.nested.owner.active);
    EXPECT_TRUE(value.nested.payload.id == intx::uint256{7});
    EXPECT_EQ(value.nested.payload.label, "inner");
    EXPECT_EQ(value.nested.note, "outer");
    ASSERT_EQ(value.history.size(), 2);
    EXPECT_TRUE(value.history[0].id == intx::uint256{1});
    EXPECT_TRUE(value.history[0].active);
    EXPECT_TRUE(value.history[1].id == intx::uint256{2});
    EXPECT_FALSE(value.history[1].active);
}

TEST(AbiDecoderComposites, DecodesNestedDynamicStructArray)
{
    const auto data = from_hex(abi_test_vectors::kNestedDynamicRecordArray2);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::dyn_array_t<NestedDynamicRecord>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    ASSERT_EQ(value.size(), 2);
    EXPECT_TRUE(value[0].owner.id == intx::uint256{1});
    EXPECT_TRUE(value[0].owner.active);
    EXPECT_TRUE(value[0].payload.id == intx::uint256{10});
    EXPECT_EQ(value[0].payload.label, "alpha");
    EXPECT_EQ(value[0].note, "first");
    EXPECT_TRUE(value[1].owner.id == intx::uint256{2});
    EXPECT_FALSE(value[1].owner.active);
    EXPECT_TRUE(value[1].payload.id == intx::uint256{20});
    EXPECT_EQ(value[1].payload.label, "beta");
    EXPECT_EQ(value[1].note, "second");
}
