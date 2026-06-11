// Copyright (c) 2026 Ivan Nazarov
// SPDX-License-Identifier: MIT

#pragma once

#include <solabi/decoder_fwd.h>

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include <intx/intx.hpp>

namespace solabi
{
namespace internal
{
inline std::optional<size_t> checked_add(size_t lhs, size_t rhs)
{
    if (rhs > static_cast<size_t>(-1) - lhs) {
        return std::nullopt;
    }
    return lhs + rhs;
}

inline bool bytes_are(bytes_view data, size_t begin, size_t end, uint8_t expected)
{
    for (size_t i = begin; i < end; ++i) {
        if (data[i] != expected) {
            return false;
        }
    }
    return true;
}

inline size_t padding_size(size_t length)
{
    return (kWordSize - (length % kWordSize)) % kWordSize;
}

inline void validate_padded_payload(bytes_view data,
                                    size_t payload,
                                    size_t length,
                                    std::string_view type_name)
{
    const auto padded_length_res = checked_add(length, padding_size(length));
    if (!padded_length_res) {
        throw std::runtime_error(std::string{"ABI decoder: '"}.append(type_name).append(
            "' padded payload length overflow"));
    }
    const size_t padded_length = *padded_length_res;

    if (payload > data.size() || padded_length > data.size() - payload) {
        throw std::runtime_error(std::string{"ABI decoder: '"}.append(type_name).append(
            "' padded payload out of range"));
    }

    const auto padding_begin_res = checked_add(payload, length);
    if (!padding_begin_res) {
        throw std::runtime_error(
            std::string{"ABI decoder: '"}.append(type_name).append("' padding offset overflow"));
    }
    const size_t padding_begin = *padding_begin_res;

    const auto padding_end_res = checked_add(payload, padded_length);
    if (!padding_end_res) {
        throw std::runtime_error(
            std::string{"ABI decoder: '"}.append(type_name).append("' padding end overflow"));
    }
    const size_t padding_end = *padding_end_res;

    if (!bytes_are(data, padding_begin, padding_end, uint8_t{0})) {
        throw std::runtime_error(
            std::string{"ABI decoder: non-zero '"}.append(type_name).append("' padding"));
    }
}

inline std::optional<size_t> read_size(bytes_view data, size_t pos)
{
    if (pos > data.size() || kWordSize > data.size() - pos) {
        return std::nullopt;
    }

    const auto word = data.substr(pos, kWordSize);
    size_t res = 0;

    for (size_t i = 0; i < kWordSize; ++i) {
        const auto byte = word[i];
        if (i < kWordSize - sizeof(size_t) && byte != 0) {
            return std::nullopt;
        }

        if (i >= kWordSize - sizeof(size_t)) {
            res = (res << 8) | byte;
        }
    }

    return res;
}

inline std::optional<bytes_view> safe_substr(bytes_view data,
                                             size_t pos = 0,
                                             size_t len = bytes_view::npos)
{
    if (pos > data.size()) {
        return std::nullopt;
    }

    if (len != bytes_view::npos && len > data.size() - pos) {
        return std::nullopt;
    }

    return data.substr(pos, len);
}

inline auto read_dynamic_payload(bytes_view data, size_t& pos, std::string_view type_name)
    -> bytes_view
{
    const auto read_offset_res = read_size(data, pos);
    if (!read_offset_res) {
        throw std::runtime_error(
            std::string{"ABI decoder: can not read '"}.append(type_name).append("' offset"));
    }
    pos += kWordSize;
    const size_t offset = *read_offset_res;

    const auto read_length_res = read_size(data, offset);
    if (!read_length_res) {
        throw std::runtime_error(
            std::string{"ABI decoder: can not read '"}.append(type_name).append("' length"));
    }
    const size_t length = *read_length_res;

    const auto payload_res = checked_add(offset, kWordSize);
    if (!payload_res) {
        throw std::runtime_error(
            std::string{"ABI decoder: '"}.append(type_name).append("' payload offset overflow"));
    }
    const size_t payload = *payload_res;

    if (payload > data.size() || length > data.size() - payload) {
        throw std::runtime_error(
            std::string{"ABI decoder: '"}.append(type_name).append("' payload out of range"));
    }
    validate_padded_payload(data, payload, length, type_name);

    return data.substr(payload, length);
}

// uint<B>
template <unsigned B>
requires ValidIntBits<B>
inline auto decode(uint_t<B>, bytes_view data, size_t& pos) -> typename cpp_type<uint_t<B>>::type
{
    const auto substr_res = safe_substr(data, pos, kWordSize);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'uint' data");
    }
    const auto word = *substr_res;
    pos += kWordSize;

    constexpr size_t kValueBytes = B / 8;
    if constexpr (kValueBytes < kWordSize) {
        if (!bytes_are(word, 0, kWordSize - kValueBytes, uint8_t{0})) {
            throw std::runtime_error(
                "ABI decoder: 'uint' value does not fit into requested bit width");
        }
    }

    const auto val =
        intx::be::load<intx::uint256>(*reinterpret_cast<const uint8_t (*)[kWordSize]>(word.data()));
    return val;
}

// int<B>
template <unsigned B>
requires ValidIntBits<B>
inline auto decode(int_t<B>, bytes_view data, size_t& pos) -> typename cpp_type<int_t<B>>::type
{
    const auto substr_res = safe_substr(data, pos, kWordSize);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'int' data");
    }
    const auto word = *substr_res;
    pos += kWordSize;

    constexpr size_t kValueBytes = B / 8;
    if constexpr (kValueBytes < kWordSize) {
        const size_t sign_byte_index = kWordSize - kValueBytes;
        const uint8_t expected_prefix =
            (word[sign_byte_index] & 0x80) != 0 ? uint8_t{0xff} : uint8_t{0};
        if (!bytes_are(word, 0, sign_byte_index, expected_prefix)) {
            throw std::runtime_error("ABI decoder: invalid 'int' sign extension");
        }
    }

    const auto val =
        intx::be::load<intx::uint256>(*reinterpret_cast<const uint8_t (*)[kWordSize]>(word.data()));
    return val;
}

// bool
inline auto decode(bool_t, bytes_view data, size_t& pos) -> typename cpp_type<bool_t>::type
{
    const auto substr_res = safe_substr(data, pos, kWordSize);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'bool' data");
    }
    const auto word = *substr_res;
    pos += kWordSize;

    if (!bytes_are(word, 0, kWordSize - 1, uint8_t{0}) || (word.back() != 0 && word.back() != 1)) {
        throw std::runtime_error("ABI decoder: invalid 'bool' value");
    }

    const bool val = word.back() == 1;
    return val;
}

// address
inline auto decode(address_t, bytes_view data, size_t& pos) -> cpp_type<address_t>::type
{
    const auto substr_res = safe_substr(data, pos, kWordSize);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'address' data");
    }
    const auto word = *substr_res;
    pos += kWordSize;

    if (!bytes_are(word, 0, kWordSize - kAddressLength, uint8_t{0})) {
        throw std::runtime_error("ABI decoder: non-zero 'address' prefix");
    }

    const auto val = bytes(word.data() + word.size() - kAddressLength, kAddressLength);
    return val;
}

// bytes<N>
template <unsigned N>
requires ValidBytesN<N>
inline auto decode(bytes_fixed_t<N>, bytes_view data, size_t& pos) ->
    typename cpp_type<bytes_fixed_t<N>>::type
{
    const auto substr_res = safe_substr(data, pos, kWordSize);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'static bytes' data");
    }
    const auto word = *substr_res;
    pos += kWordSize;

    if constexpr (N < kWordSize) {
        if (!bytes_are(word, N, kWordSize, uint8_t{0})) {
            throw std::runtime_error("ABI decoder: non-zero 'static bytes' padding");
        }
    }

    const auto val = bytes(word.data(), N);
    return val;
}

// bytes
inline auto decode(bytes_dyn_t, bytes_view data, size_t& pos) -> cpp_type<bytes_dyn_t>::type
{
    const auto payload = read_dynamic_payload(data, pos, "bytes");
    const auto val = bytes(payload.data(), payload.size());
    return val;
}

// string
inline auto decode(string_t, bytes_view data, size_t& pos) -> cpp_type<string_t>::type
{
    const auto payload = read_dynamic_payload(data, pos, "string");
    const auto val = std::string(payload.begin(), payload.end());
    return val;
}

// User-defined C++ type with `abi_tag`
template <HasAbiTupleTag T>
inline auto decode(T, bytes_view data, size_t& pos) -> typename cpp_type<T>::type
{
    using Tag = typename T::abi_tag;
    auto tup = decode(Tag{}, data, pos);
    return from_tuple<T>(std::move(tup));
}

// <T>[N]: fixed-length array of elements of the given type
template <class T, size_t N>
inline auto decode(array_t<T, N>, bytes_view data, size_t& pos) ->
    typename cpp_type<array_t<T, N>>::type
{
    typename cpp_type<array_t<T, N>>::type val{};

    if constexpr (kIsStatic<T>) {
        for (auto& item : val) {
            item = decode(T{}, data, pos);
        }
    } else {
        // Dynamic fixed array: its head contains an offset to tuple-like array data.
        const auto read_offset_res = read_size(data, pos);
        if (!read_offset_res) {
            throw std::runtime_error(
                "ABI decoder: can not read offset to 'fixed-length array' data");
        }
        pos += kWordSize;
        size_t offset = *read_offset_res;

        const auto substr_res = safe_substr(data, offset);
        if (!substr_res) {
            throw std::runtime_error("ABI decoder: can not read 'fixed-length array' data");
        }
        const auto array_data = *substr_res;

        size_t local_pos = 0;
        for (auto& item : val) {
            item = decode(T{}, array_data, local_pos);
        }
    }

    return val;
}

// <T>[]: variable-length array of elements of the given type
template <class T>
inline auto decode(dyn_array_t<T>, bytes_view data, size_t& pos) ->
    typename cpp_type<dyn_array_t<T>>::type
{
    // Get offset
    const auto read_offset_res = read_size(data, pos);
    if (!read_offset_res) {
        throw std::runtime_error(
            "ABI decoder: can not read offset to 'variable-length array' data");
    }
    pos += kWordSize;
    size_t offset = *read_offset_res;

    // Get array length
    const auto read_length_res = read_size(data, offset);
    if (!read_length_res) {
        throw std::runtime_error("ABI decoder: can not read 'variable-length array' length");
    }
    const size_t length = *read_length_res;

    // Get array data
    const auto array_data_offset_res = checked_add(offset, kWordSize);
    if (!array_data_offset_res) {
        throw std::runtime_error("ABI decoder: 'variable-length array' data offset overflow");
    }
    const size_t array_data_offset = *array_data_offset_res;
    const auto substr_res = safe_substr(data, array_data_offset);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'variable-length array' data");
    }
    auto array_data = *substr_res;

    typename cpp_type<dyn_array_t<T>>::type val;
    val.reserve(length);

    size_t local_pos = 0;
    for (size_t i = 0; i < length; ++i) {
        val.emplace_back(decode(T{}, array_data, local_pos));
    }
    return val;
}

// (T1, T2,...,Tn): tuple consisting of the types T1, ..., Tn, n >= 0
template <class... Ts>
inline auto decode(tuple_t<Ts...>, bytes_view data, size_t& pos) ->
    typename cpp_type<tuple_t<Ts...>>::type
{
    if constexpr (std::bool_constant<(kIsStatic<Ts> && ...)>::value) {
        // Static tuple: its components are laid out sequentially in the head
        typename cpp_type<tuple_t<Ts...>>::type val{};

        auto fill = [&](auto seq) {
            using Seq = decltype(seq);
            return ([&]<size_t... Is>(std::index_sequence<Is...>) {
                ((std::get<Is>(val) = decode(Ts{}, data, pos)), ...);
            })(Seq{});
        };

        fill(std::index_sequence_for<Ts...>{});
        return val;
    } else {
        // Dynamic tuple: the head at the tuple's position contains an offset
        const auto read_offset_res = read_size(data, pos);
        if (!read_offset_res) {
            throw std::runtime_error("ABI decoder: can not read 'tuple' offset");
        }
        pos += kWordSize;
        size_t offset = *read_offset_res;

        // Get tuple data
        const auto substr_res = safe_substr(data, offset);
        if (!substr_res) {
            throw std::runtime_error("ABI decoder: can not read dynamic tuple data");
        }
        const auto tuple_data = *substr_res;

        // Decode from the tail, starting at the tuple's local head
        size_t local = 0;
        const auto val =
            std::tuple<typename cpp_type<Ts>::type...>{decode(Ts{}, tuple_data, local)...};
        return val;
    }
}

template <AbiTag Tag>
inline auto decode_at(Tag tag, bytes_view data, size_t at) -> typename cpp_type<Tag>::type
{
    size_t local = at;
    return decode(tag, data, local);
}

} // namespace internal

template <AbiTag Tag>
inline auto decode(bytes_view data, size_t& pos) -> typename cpp_type<Tag>::type
{
    if (pos % kWordSize != 0) {
        throw std::runtime_error("ABI decoder: head position must be 32-byte aligned");
    }

    return internal::decode(Tag{}, data, pos);
}

template <AbiTag Tag>
inline auto decode(bytes_view data) -> typename cpp_type<Tag>::type
{
    return internal::decode_at(Tag{}, data, 0);
}

template <HasAbiTupleTag T>
inline auto from_tuple(typename ::solabi::cpp_type<typename T::abi_tag>::type tup) -> T
{
    return std::apply(
        [](auto&&... xs) {
            return T{std::forward<decltype(xs)>(xs)...};
        },
        std::move(tup));
}

} // namespace solabi
