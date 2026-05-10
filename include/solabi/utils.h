// Copyright (c) 2026 Ivan Nazarov
// SPDX-License-Identifier: MIT

#pragma once

#include <solabi/common.h>

#include <cstddef>
#include <stdexcept>
#include <string_view>

namespace solabi
{
namespace internal
{
inline int hex_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    throw std::invalid_argument("solabi: invalid hex character");
}
} // namespace internal

inline bytes from_hex(std::string_view hex)
{
    if (hex.starts_with("0x") || hex.starts_with("0X")) {
        hex.remove_prefix(2);
    }
    if (hex.size() % 2 != 0) {
        throw std::invalid_argument("solabi: hex string must contain an even number of digits");
    }

    bytes data;
    data.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        data.push_back(static_cast<uint8_t>((internal::hex_value(hex[i]) << 4) |
                                            internal::hex_value(hex[i + 1])));
    }
    return data;
}
} // namespace solabi
