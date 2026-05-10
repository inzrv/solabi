// Copyright (c) 2026 Ivan Nazarov
// SPDX-License-Identifier: MIT

#include "test_types.h"

#include <array>
#include <concepts>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

template <class T, class Tuple, size_t... Is>
constexpr bool brace_constructible_from_tuple(std::index_sequence<Is...>)
{
    return requires { T{std::declval<std::tuple_element_t<Is, Tuple>>()...}; };
}

template <class T>
constexpr bool is_currently_usable_user_abi_type()
{
    if constexpr (!solabi::HasAbiTupleTag<T>) {
        return false;
    } else {
        using Tuple = solabi::cpp_type<typename T::abi_tag>::type;
        return std::default_initializable<T> &&
               brace_constructible_from_tuple<T, Tuple>(
                   std::make_index_sequence<std::tuple_size_v<Tuple>>{});
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
static_assert(
    std::is_same_v<solabi::cpp_type<solabi::dyn_array_t<solabi::bool_t>>::type, std::vector<bool>>);
