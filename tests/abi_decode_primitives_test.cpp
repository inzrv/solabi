#include <solabi/decoder.h>
#include <solabi/utils.h>
#include "abi_vectors.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <string>

namespace
{
using solabi::from_hex;
} // namespace

TEST(AbiDecoderPrimitives, DecodesUintWord)
{
    const auto data = from_hex(abi_test_vectors::kUint256_42);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::uint_t<256>>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value == intx::uint256{42});
}

TEST(AbiDecoderPrimitives, RejectsTruncatedStaticWord)
{
    const auto data = from_hex("00");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::uint_t<256>>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesUintSizes)
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

TEST(AbiDecoderPrimitives, RejectsUintThatDoesNotFitBitWidth)
{
    const auto data = from_hex(
        "0000000000000000000000000000000000000000000000000000000000000100");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::uint_t<8>>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesSignedIntsAsTwosComplement)
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

TEST(AbiDecoderPrimitives, RejectsInvalidIntSignExtension)
{
    const auto data = from_hex(
        "00000000000000000000000000000000000000000000000000000000000000fb");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::int_t<8>>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesBoolWord)
{
    const auto data = from_hex(abi_test_vectors::kBoolTrue);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::bool_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_TRUE(value);
}

TEST(AbiDecoderPrimitives, DecodesBoolFalse)
{
    const auto data = from_hex(abi_test_vectors::kBoolFalse);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::bool_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_FALSE(value);
}

TEST(AbiDecoderPrimitives, RejectsInvalidBoolEncoding)
{
    const auto data = from_hex(
        "0000000000000000000000000000000000000000000000000000000000000002");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::bool_t>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesAddress)
{
    const auto data = from_hex(abi_test_vectors::kAddress);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::address_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value, from_hex("1111111111111111111111111111111111111111"));
}

TEST(AbiDecoderPrimitives, RejectsAddressWithNonZeroPrefix)
{
    const auto data = from_hex(
        "0100000000000000000000001111111111111111111111111111111111111111");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::address_t>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesFixedBytes)
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

TEST(AbiDecoderPrimitives, RejectsFixedBytesWithNonZeroPadding)
{
    const auto data = from_hex(
        "abcdef0000000000000000000000000000000000000000000000000000000001");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::bytes_fixed_t<3>>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesDynamicBytes)
{
    const auto data = from_hex(abi_test_vectors::kBytesDynamic);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::bytes_dyn_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value, from_hex("12345678"));
}

TEST(AbiDecoderPrimitives, RejectsDynamicBytesWithNonZeroPadding)
{
    const auto data = from_hex(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000004"
        "1234567800000000000000000000000000000000000000000000000000000001");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::bytes_dyn_t>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesEmptyDynamicValues)
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

TEST(AbiDecoderPrimitives, RejectsDynamicBytesWithOffsetOverflow)
{
    const auto data = from_hex(
        "000000000000000000000000000000000000000000000000fffffffffffffff0");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::bytes_dyn_t>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, RejectsDynamicBytesWithPayloadLengthOverflow)
{
    const auto data = from_hex(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "000000000000000000000000000000000000000000000000fffffffffffffff0");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::bytes_dyn_t>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}

TEST(AbiDecoderPrimitives, DecodesString)
{
    const auto data = from_hex(abi_test_vectors::kStringHi);

    size_t pos = 0;
    const auto value = solabi::decode<solabi::string_t>(bytes_view{data.data(), data.size()}, pos);

    EXPECT_EQ(pos, WORD_SIZE);
    EXPECT_EQ(value, "hi");
}

TEST(AbiDecoderPrimitives, RejectsStringWithNonZeroPadding)
{
    const auto data = from_hex(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000002"
        "6869000000000000000000000000000000000000000000000000000000000001");

    size_t pos = 0;
    EXPECT_THROW((solabi::decode<solabi::string_t>(bytes_view{data.data(), data.size()}, pos)),
                 std::runtime_error);
}
