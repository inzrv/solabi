#pragma once

#include "decoder_fwd.h"

#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include <intx/intx.hpp>

namespace solabi
{
namespace internal
{
inline std::optional<size_t> read_size(bytes_view data, size_t pos)
{
    if (pos > data.size() || WORD_SIZE > data.size() - pos) {
        return std::nullopt;
    }

    const auto word = data.substr(pos, WORD_SIZE);
    size_t res = 0;

    for (size_t i = 0; i < WORD_SIZE; ++i) {
        const auto byte = word[i];
        if (i < WORD_SIZE - sizeof(size_t) && byte != 0) {
            return std::nullopt;
        }

        if (i >= WORD_SIZE - sizeof(size_t)) {
            res = (res << 8) | byte;
        }
    }

    return res;
}

inline std::optional<bytes_view> safe_substr(bytes_view data, size_t pos = 0, size_t len = bytes_view::npos)
{
    if (pos > data.size()) {
        return std::nullopt;
    }

    if (len != bytes_view::npos && pos + len > data.size()) {
        return std::nullopt;
    }

    return data.substr(pos, len);
}

// uint<B>
template <unsigned B>
requires ValidIntBits<B>
inline auto decode(uint_t<B>, bytes_view data, size_t& pos)
    -> typename cpp_type<uint_t<B>>::type
{
    const auto substr_res = safe_substr(data, pos, WORD_SIZE);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'uint' data");
    }
    const auto word = *substr_res;
    pos += WORD_SIZE;

    const auto val = intx::be::load<intx::uint256>(*reinterpret_cast<const uint8_t(*)[WORD_SIZE]>(word.data()));
    return val;
}

// int<B>
template <unsigned B>
requires ValidIntBits<B>
inline auto decode(int_t<B>, bytes_view data, size_t& pos)
    -> typename cpp_type<int_t<B>>::type
{
    return decode(uint_t<B>{}, data, pos);
}

// bool
inline auto decode(bool_t, bytes_view data, size_t& pos)
    -> typename cpp_type<bool_t>::type
{
    const auto substr_res = safe_substr(data, pos, WORD_SIZE);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'bool' data");
    }
    const auto word = *substr_res;
    pos += WORD_SIZE;

    const bool val = word.back() == 0x0 ? false : true;
    return val;
}

// address
inline auto decode(address_t, bytes_view data, size_t& pos)
    -> cpp_type<address_t>::type
{
    const auto substr_res = safe_substr(data, pos, WORD_SIZE);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'address' data");
    }
    const auto word = *substr_res;
    pos += WORD_SIZE;

    const auto val = bytes(word.data() + word.size() - ADDRESS_LENGTH, ADDRESS_LENGTH);
    return val;
}

// bytes<N>
template<unsigned N>
requires ValidBytesN<N>
inline auto decode(bytes_fixed_t<N>, bytes_view data, size_t& pos)
    -> typename cpp_type<bytes_fixed_t<N>>::type
{
    const auto substr_res = safe_substr(data, pos, WORD_SIZE);
    if (!substr_res) {
        throw std::runtime_error("ABI decoder: can not read 'static bytes' data");
    }
    const auto word = *substr_res;
    pos += WORD_SIZE;

    const auto val = bytes(word.data(), N);
    return val;
}

// bytes
inline auto decode(bytes_dyn_t, bytes_view data, size_t& pos)
    -> cpp_type<bytes_dyn_t>::type
{
    // Get offset
    const auto read_offset_res = read_size(data, pos);
    if (!read_offset_res) {
        throw std::runtime_error("ABI decoder: can not read 'bytes' offset");
    }
    pos += WORD_SIZE;
    size_t offset = *read_offset_res;

     // Get bytes length
    const auto read_length_res = read_size(data, offset);
    if (!read_length_res) {
        throw std::runtime_error("Can not read 'bytes' length");
    }
    const size_t length = *read_length_res;
    const size_t payload = offset + WORD_SIZE;

    if (payload + length > data.size()) {
        throw std::runtime_error("ABI decoder: 'bytes' payload out of range");
    }

    const auto val = bytes(data.data() + payload, length);
    return val;
}

// string
inline auto decode(string_t, bytes_view data, size_t& pos)
    -> cpp_type<string_t>::type
{
    // Get offset
    const auto read_offset_res = read_size(data, pos);
    if (!read_offset_res) {
        throw std::runtime_error("ABI decoder: can not read 'string' offset");
    }
    pos += WORD_SIZE;
    size_t offset = *read_offset_res;

     // Get string length
    const auto read_length_res = read_size(data, offset);
    if (!read_length_res) {
        throw std::runtime_error("Can not read 'string' length");
    }
    const size_t length = *read_length_res;

    const size_t payload = offset + WORD_SIZE;

    if (payload + length > data.size()) {
        throw std::runtime_error("ABI decoder: 'string' payload out of range");
    }

    const auto first = data.begin() + payload;
    const auto last = first + length;

    const auto val = std::string(first, last);
    return val;
}

// User-defined C++ type with `abi_tag`
template<HasAbiTupleTag T>
inline auto decode(T, bytes_view data, size_t& pos)
    -> typename cpp_type<T>::type
{
    using Tag = typename T::abi_tag;
    auto tup = decode(Tag{}, data, pos);
    return from_tuple<T>(std::move(tup));
}

// <T>[N]: fixed-length array of elements of the given type
template<class T, size_t N>
inline auto decode(array_t<T, N>, bytes_view data, size_t& pos)
    -> typename cpp_type<array_t<T, N>>::type
{
    typename cpp_type<array_t<T, N>>::type val{};

    if constexpr (is_static_v<T>) {
        for (auto& item : val) {
            item = decode(T{}, data, pos);
        }
    } else {
        // Dynamic fixed array: its head contains an offset to tuple-like array data.
        const auto read_offset_res = read_size(data, pos);
        if (!read_offset_res) {
            throw std::runtime_error("ABI decoder: can not read offset to 'fixed-length array' data");
        }
        pos += WORD_SIZE;
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
template<class T>
inline auto decode(dyn_array_t<T>, bytes_view data, size_t& pos)
    -> typename cpp_type<dyn_array_t<T>>::type
{
    // Get offset
    const auto read_offset_res = read_size(data, pos);
    if (!read_offset_res) {
        throw std::runtime_error("ABI decoder: can not read offset to 'variable-length array' data");
    }
    pos += WORD_SIZE;
    size_t offset = *read_offset_res;

    // Get array length
    const auto read_length_res = read_size(data, offset);
    if (!read_length_res) {
        throw std::runtime_error("ABI decoder: can not read 'variable-length array' length");
    }
    const size_t length = *read_length_res;

    // Get array data
    const size_t array_data_offset = offset + WORD_SIZE;
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
template<class... Ts>
inline auto decode(tuple_t<Ts...>, bytes_view data, size_t& pos)
    -> typename cpp_type<tuple_t<Ts...>>::type
{
    if constexpr (std::bool_constant<(is_static_v<Ts> && ...)>::value) {
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
        pos += WORD_SIZE;
        size_t offset = *read_offset_res;

        // Get tuple data
        const auto substr_res = safe_substr(data, offset);
        if (!substr_res) {
            throw std::runtime_error("ABI decoder: can not read dynamic tuple data");
        }
        const auto tuple_data = *substr_res;

        // Decode from the tail, starting at the tuple's local head
        size_t local = 0;
        const auto val = std::tuple<typename cpp_type<Ts>::type...>{
            decode(Ts{}, tuple_data, local)...
        };
        return val;
    }
}

template<AbiTag Tag>
inline auto decode_at(Tag tag, bytes_view data, size_t at)
    -> typename cpp_type<Tag>::type
{
    size_t local = at;
    return decode(tag, data, local);
}

} // namespace internal

template<AbiTag Tag>
inline auto decode(bytes_view data, size_t& pos)
    -> typename cpp_type<Tag>::type
{
    if (pos % WORD_SIZE != 0) {
        throw std::runtime_error("ABI decoder: head position must be 32-byte aligned");
    }

    return internal::decode(Tag{}, data, pos);
}

template<AbiTag Tag>
inline auto decode(bytes_view data)
    -> typename cpp_type<Tag>::type
{
    return internal::decode_at(Tag{}, data, 0);
}

template<HasAbiTupleTag T>
inline auto from_tuple(typename ::solabi::cpp_type<typename T::abi_tag>::type tup)
    -> T
{
    return std::apply([](auto&&... xs) {
        return T{ std::forward<decltype(xs)>(xs)... };
    }, std::move(tup));
}

template<class T>
requires HasAbiTupleTag<T>
inline auto decode_as(bytes_view data, size_t& pos)
    -> T
{
    using Tag = typename T::abi_tag;
    auto tup = ::solabi::decode<Tag>(data, pos);
    return from_tuple<T>(std::move(tup));
}

} // namespace solabi
