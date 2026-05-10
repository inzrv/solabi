#pragma once

#include <cstdint>
#include <string>
#include <string_view>

constexpr unsigned int WORD_SIZE = 32;
constexpr unsigned int ADDRESS_LENGTH = 20;

using bytes = std::basic_string<uint8_t>;
using bytes_view = std::basic_string_view<uint8_t>;
