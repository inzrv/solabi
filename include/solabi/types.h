// Copyright (c) 2026 Ivan Nazarov
// SPDX-License-Identifier: MIT

#pragma once

#include <solabi/common.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <intx/intx.hpp>

namespace solabi
{
// Concepts

template <unsigned Bits>
concept ValidIntBits = (Bits % 8 == 0) && (Bits >= 8 && Bits <= 256);

template <unsigned N>
concept ValidBytesN = (N >= 1 && N <= 32);

// Type tags

template <unsigned Bits>
requires ValidIntBits<Bits>
struct uint_t
{
    static constexpr unsigned bits = Bits;
};

template <unsigned Bits>
requires ValidIntBits<Bits>
struct int_t
{
    static constexpr unsigned bits = Bits;
};

struct bool_t
{};

struct address_t
{};

template <unsigned N>
requires ValidBytesN<N>
struct bytes_fixed_t
{
    static constexpr unsigned size = N;
};

struct bytes_dyn_t
{};

struct string_t
{};

template <class T, size_t N>
struct array_t
{
    using element = T;
    static constexpr size_t length = N;
};

template <class T>
struct dyn_array_t
{
    using element = T;
};

template <class... Ts>
struct tuple_t
{
    using elements = std::tuple<Ts...>;
    static constexpr size_t size = sizeof...(Ts);
};

// Carrier types

template <class Sol>
struct cpp_type;

template <class T>
concept HasAbiTag = requires { typename T::abi_tag; };

template <class Tag>
struct is_tuple_tag : std::false_type
{};

template <class... Ts>
struct is_tuple_tag<tuple_t<Ts...>> : std::true_type
{};

template <class T, bool = HasAbiTag<T>>
struct has_abi_tuple_tag : std::false_type
{};

template <class T>
struct has_abi_tuple_tag<T, true>
{
    static constexpr bool value = is_tuple_tag<typename T::abi_tag>::value &&
                                  requires { typename cpp_type<typename T::abi_tag>::type; };
};

template <class T>
concept HasAbiTupleTag = has_abi_tuple_tag<T>::value;

template <unsigned B>
requires ValidIntBits<B>
struct cpp_type<uint_t<B>>
{
    using type = intx::uint256;
};

template <unsigned B>
requires ValidIntBits<B>
struct cpp_type<int_t<B>>
{
    // Two’s complement representation
    using type = intx::uint256;
};

template <>
struct cpp_type<bool_t>
{
    using type = bool;
};

template <>
struct cpp_type<address_t>
{
    using type = bytes;
};

template <unsigned N>
requires ValidBytesN<N>
struct cpp_type<bytes_fixed_t<N>>
{
    using type = bytes;
};

template <>
struct cpp_type<bytes_dyn_t>
{
    using type = bytes;
};

template <>
struct cpp_type<string_t>
{
    using type = std::string;
};

template <class T, size_t N>
struct cpp_type<array_t<T, N>>
{
    using type = std::array<typename cpp_type<T>::type, N>;
};

template <class T>
struct cpp_type<dyn_array_t<T>>
{
    using type = std::vector<typename cpp_type<T>::type>;
};

template <class... Ts>
struct cpp_type<tuple_t<Ts...>>
{
    using type = std::tuple<typename cpp_type<Ts>::type...>;
};

template <HasAbiTupleTag T>
struct cpp_type<T>
{
    using type = T;
};

// Utils

template <class Tag>
struct is_static : std::false_type
{};

template <unsigned B>
requires ValidIntBits<B>
struct is_static<uint_t<B>> : std::true_type
{};

template <unsigned B>
requires ValidIntBits<B>
struct is_static<int_t<B>> : std::true_type
{};

template <>
struct is_static<bool_t> : std::true_type
{};

template <>
struct is_static<address_t> : std::true_type
{};

template <unsigned N>
requires ValidBytesN<N>
struct is_static<bytes_fixed_t<N>> : std::true_type
{};

template <>
struct is_static<bytes_dyn_t> : std::false_type
{};

template <>
struct is_static<string_t> : std::false_type
{};

template <class E, size_t N>
struct is_static<array_t<E, N>> : is_static<E>
{};

template <class E>
struct is_static<dyn_array_t<E>> : std::false_type
{};

template <class... Ts>
struct is_static<tuple_t<Ts...>> : std::bool_constant<(is_static<Ts>::value && ...)>
{};

template <HasAbiTupleTag T>
struct is_static<T> : is_static<typename T::abi_tag>
{};

template <class Tag>
inline constexpr bool is_static_v = is_static<Tag>::value;

// Common concept for ABI type tags
template <class T>
concept AbiTag = requires { typename cpp_type<T>::type; };

} // namespace solabi
