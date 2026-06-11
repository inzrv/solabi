// Copyright (c) 2026 Ivan Nazarov
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace solabi
{

constexpr unsigned int kWordSize = 32;
constexpr unsigned int kAddressLength = 20;

using bytes = std::basic_string<uint8_t>;
using bytes_view = std::basic_string_view<uint8_t>;

} // namespace solabi
