// Copyright (c) 2026 Ivan Nazarov
// SPDX-License-Identifier: MIT

#include <solabi/utils.h>

#include <gtest/gtest.h>

#include <stdexcept>

TEST(AbiUtils, FromHexAcceptsPrefixesAndMixedCase)
{
    EXPECT_EQ(solabi::from_hex("0xAbCd"), solabi::from_hex("abcd"));
    EXPECT_EQ(solabi::from_hex("0X1234"), solabi::from_hex("1234"));
}

TEST(AbiUtils, FromHexRejectsMalformedInput)
{
    EXPECT_THROW(solabi::from_hex("abc"), std::invalid_argument);
    EXPECT_THROW(solabi::from_hex("0xzz"), std::invalid_argument);
}
