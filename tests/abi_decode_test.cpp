#include "decoder.h"
#include "utils.h"
#include "abi_vectors.h"

#include <gtest/gtest.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>

struct StaticRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>;

    intx::uint256 id;
    bool active;
};

struct DynamicRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::string_t>;

    intx::uint256 id;
    std::string label;
};

struct NestedStaticRecord
{
    using abi_tag = solabi::tuple_t<StaticRecord, solabi::uint_t<256>>;

    StaticRecord owner;
    intx::uint256 score;
};

struct NestedDynamicRecord
{
    using abi_tag = solabi::tuple_t<StaticRecord, DynamicRecord, solabi::string_t>;

    StaticRecord owner;
    DynamicRecord payload;
    std::string note;
};

struct DeepRecord
{
    using abi_tag = solabi::tuple_t<NestedDynamicRecord, solabi::dyn_array_t<StaticRecord>>;

    NestedDynamicRecord nested;
    std::vector<StaticRecord> history;
};

struct ConstructorRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::string_t>;

    ConstructorRecord() = default;
    ConstructorRecord(intx::uint256 id_, std::string label_)
        : id{id_}, label{std::move(label_)}
    {}

    intx::uint256 id;
    std::string label;
};

struct MissingAbiTag
{
    int value{};
};

struct InvalidAbiTag
{
    using abi_tag = int;

    int value{};
};

struct PrimitiveAbiTag
{
    using abi_tag = solabi::uint_t<256>;

    intx::uint256 value{};
};

struct WrongFieldCountRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>;

    intx::uint256 id;
};

struct NoDefaultRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>;

    NoDefaultRecord(intx::uint256 id_, bool active_)
        : id{id_}, active{active_}
    {}

    intx::uint256 id;
    bool active;
};

template<class T, class Tuple, size_t... Is>
constexpr bool brace_constructible_from_tuple(std::index_sequence<Is...>)
{
    return requires {
        T{std::declval<std::tuple_element_t<Is, Tuple>>()...};
    };
}

template<class T>
constexpr bool is_currently_usable_user_abi_type()
{
    if constexpr (!solabi::HasAbiTupleTag<T>) {
        return false;
    } else {
        using Tuple = solabi::cpp_type<typename T::abi_tag>::type;
        return std::default_initializable<T> &&
            brace_constructible_from_tuple<T, Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }
}

static_assert(solabi::AbiTag<solabi::uint_t<256>>);
static_assert(solabi::AbiTag<solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>>);
static_assert(solabi::AbiTag<StaticRecord>);
static_assert(solabi::AbiTag<DynamicRecord>);
static_assert(solabi::AbiTag<NestedStaticRecord>);
static_assert(solabi::AbiTag<NestedDynamicRecord>);
static_assert(solabi::AbiTag<DeepRecord>);
static_assert(solabi::AbiTag<ConstructorRecord>);
static_assert(solabi::HasAbiTag<StaticRecord>);
static_assert(solabi::HasAbiTupleTag<StaticRecord>);
static_assert(!solabi::HasAbiTag<MissingAbiTag>);
static_assert(solabi::HasAbiTag<InvalidAbiTag>);
static_assert(!solabi::HasAbiTupleTag<InvalidAbiTag>);
static_assert(!solabi::AbiTag<InvalidAbiTag>);
static_assert(solabi::HasAbiTag<PrimitiveAbiTag>);
static_assert(!solabi::HasAbiTupleTag<PrimitiveAbiTag>);
static_assert(!solabi::AbiTag<PrimitiveAbiTag>);
static_assert(is_currently_usable_user_abi_type<StaticRecord>());
static_assert(is_currently_usable_user_abi_type<DynamicRecord>());
static_assert(is_currently_usable_user_abi_type<ConstructorRecord>());
static_assert(!is_currently_usable_user_abi_type<WrongFieldCountRecord>());
static_assert(!is_currently_usable_user_abi_type<NoDefaultRecord>());
static_assert(solabi::is_static_v<solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>>);
static_assert(solabi::is_static_v<StaticRecord>);
static_assert(solabi::is_static_v<NestedStaticRecord>);
static_assert(!solabi::is_static_v<DynamicRecord>);
static_assert(!solabi::is_static_v<NestedDynamicRecord>);
static_assert(!solabi::is_static_v<DeepRecord>);
static_assert(solabi::is_static_v<solabi::array_t<solabi::uint_t<256>, 3>>);
static_assert(!solabi::is_static_v<solabi::array_t<solabi::string_t, 2>>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::string_t>::type, std::string>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::address_t>::type, bytes>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::bytes_fixed_t<3>>::type, bytes>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::bytes_dyn_t>::type, bytes>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::array_t<solabi::uint_t<256>, 3>>::type,
                             std::array<intx::uint256, 3>>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::dyn_array_t<DynamicRecord>>::type,
                             std::vector<DynamicRecord>>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::dyn_array_t<NestedDynamicRecord>>::type,
                             std::vector<NestedDynamicRecord>>);
static_assert(std::is_same_v<solabi::cpp_type<solabi::dyn_array_t<solabi::bool_t>>::type, std::vector<bool>>);

namespace
{
using solabi::from_hex;
} // namespace

TEST(AbiDecoder, DecodesUintWord)
{
    const auto data = from_hex(abi_test_vectors::kUint256_42);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::uint_t<256>>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value == intx::uint256{42});
}

TEST(AbiDecoder, FromHexAcceptsPrefixesAndMixedCase)
{
    EXPECT_EQ(from_hex("0xAbCd"), from_hex("abcd"));
    EXPECT_EQ(from_hex("0X1234"), from_hex("1234"));
}

TEST(AbiDecoder, FromHexRejectsMalformedInput)
{
    EXPECT_THROW(from_hex("abc"), std::invalid_argument);
    EXPECT_THROW(from_hex("0xzz"), std::invalid_argument);
}

TEST(AbiDecoder, DecodesUintSizes)
{
    {
        const auto data = from_hex(abi_test_vectors::kUint8Max);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::uint_t<8>>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_TRUE(value == intx::uint256{255});
    }
    {
        const auto data = from_hex(abi_test_vectors::kUint64Pattern);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::uint_t<64>>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_TRUE(value == intx::uint256{0x1122334455667788ULL});
    }
}

TEST(AbiDecoder, DecodesSignedIntsAsTwosComplement)
{
    {
        const auto data = from_hex(abi_test_vectors::kInt256Minus1);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::int_t<256>>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_EQ(intx::to_string(value, 16), std::string(64, 'f'));
    }
    {
        const auto data = from_hex(abi_test_vectors::kInt8Minus5);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::int_t<8>>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_EQ(intx::to_string(value, 16), std::string(62, 'f') + "fb");
    }
}

TEST(AbiDecoder, DecodesBoolWord)
{
    const auto data = from_hex(abi_test_vectors::kBoolTrue);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::bool_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value);
}

TEST(AbiDecoder, DecodesBoolFalse)
{
    const auto data = from_hex(abi_test_vectors::kBoolFalse);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::bool_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_FALSE(value);
}

TEST(AbiDecoder, DecodesAddress)
{
    const auto data = from_hex(abi_test_vectors::kAddress);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::address_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value, from_hex("1111111111111111111111111111111111111111"));
}

TEST(AbiDecoder, DecodesFixedBytes)
{
    {
        const auto data = from_hex(abi_test_vectors::kBytes3);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::bytes_fixed_t<3>>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_EQ(value, from_hex("abcdef"));
    }
    {
        const auto data = from_hex(abi_test_vectors::kBytes32);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::bytes_fixed_t<32>>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_EQ(value, from_hex("000000000000000000000000000000000000000000000000000000000000007f"));
    }
}

TEST(AbiDecoder, DecodesDynamicBytes)
{
    const auto data = from_hex(abi_test_vectors::kBytesDynamic);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::bytes_dyn_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value, from_hex("12345678"));
}

TEST(AbiDecoder, DecodesEmptyDynamicValues)
{
    {
        const auto data = from_hex(abi_test_vectors::kBytesDynamicEmpty);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::bytes_dyn_t>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_TRUE(value.empty());
    }
    {
        const auto data = from_hex(abi_test_vectors::kStringEmpty);

        size_t pos = 0;
        const auto value = solabi::decode<solabi::string_t>(bytes_view{data.data(), data.size()}, pos);

        EXPECT_EQ(pos, WORD_SIZE);
        EXPECT_TRUE(value.empty());
    }
}

TEST(AbiDecoder, DecodesString)
{
    const auto data = from_hex(abi_test_vectors::kStringHi);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::string_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value, "hi");
}

TEST(AbiDecoder, DecodesStaticFixedArray)
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

TEST(AbiDecoder, DecodesDynamicFixedArray)
{
    const auto data = from_hex(abi_test_vectors::kStringArray2);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<solabi::string_t, 2>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value[0], "hi");
    EXPECT_EQ(value[1], "bye");
}

TEST(AbiDecoder, DecodesDynamicArray)
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

TEST(AbiDecoder, DecodesDynamicTuple)
{
    const auto data = from_hex(abi_test_vectors::kTupleUint256String);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::tuple_t<solabi::uint_t<256>, solabi::string_t>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(std::get<0>(value) == intx::uint256{7});
    EXPECT_EQ(std::get<1>(value), "hi");
}

TEST(AbiDecoder, DecodesStaticStruct)
{
    const auto data = from_hex(abi_test_vectors::kStaticRecord);

    size_t pos = 0;
    const auto value = solabi::decode_as<StaticRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 2);
    EXPECT_TRUE(value.id == intx::uint256{9});
    EXPECT_TRUE(value.active);
}

TEST(AbiDecoder, DecodesDynamicStructAsAbiTag)
{
    const auto data = from_hex(abi_test_vectors::kTupleUint256String);

    size_t pos = 0;
    const auto value = solabi::decode<DynamicRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value.id == intx::uint256{7});
    EXPECT_EQ(value.label, "hi");
}

TEST(AbiDecoder, DecodesUserDefinedTypeWithConstructor)
{
    const auto data = from_hex(abi_test_vectors::kTupleUint256String);

    size_t pos = 0;
    const auto value = solabi::decode<ConstructorRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value.id == intx::uint256{7});
    EXPECT_EQ(value.label, "hi");
}

TEST(AbiDecoder, DecodesStringDynamicArray)
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

TEST(AbiDecoder, DecodesStaticStructFixedArray)
{
    const auto data = from_hex(abi_test_vectors::kStaticRecordArray2);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::array_t<StaticRecord, 2>>(
        bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 4);
    EXPECT_TRUE(value[0].id == intx::uint256{1});
    EXPECT_TRUE(value[0].active);
    EXPECT_TRUE(value[1].id == intx::uint256{2});
    EXPECT_FALSE(value[1].active);
}

TEST(AbiDecoder, DecodesDynamicStructFixedArray)
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

TEST(AbiDecoder, DecodesDynamicStructDynamicArray)
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

TEST(AbiDecoder, DecodesDynamicArrayOfFixedArrays)
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

TEST(AbiDecoder, DecodesFixedArrayOfDynamicArrays)
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

TEST(AbiDecoder, DecodesNestedStaticStruct)
{
    const auto data = from_hex(abi_test_vectors::kNestedStaticRecord);

    size_t pos = 0;
    const auto value = solabi::decode<NestedStaticRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE * 3);
    EXPECT_TRUE(value.owner.id == intx::uint256{9});
    EXPECT_TRUE(value.owner.active);
    EXPECT_TRUE(value.score == intx::uint256{11});
}

TEST(AbiDecoder, DecodesNestedDynamicStruct)
{
    const auto data = from_hex(abi_test_vectors::kNestedDynamicRecord);

    size_t pos = 0;
    const auto value = solabi::decode<NestedDynamicRecord>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value.owner.id == intx::uint256{9});
    EXPECT_TRUE(value.owner.active);
    EXPECT_TRUE(value.payload.id == intx::uint256{7});
    EXPECT_EQ(value.payload.label, "inner");
    EXPECT_EQ(value.note, "outer");
}

TEST(AbiDecoder, DecodesDeepNestedStruct)
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

TEST(AbiDecoder, DecodesNestedDynamicStructArray)
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
