#pragma once

#include <solabi/types.h>

namespace solabi
{
namespace internal
{

// uint<B>
template <unsigned B>
requires ValidIntBits<B>
inline auto decode(uint_t<B>, bytes_view data, size_t& pos) -> typename cpp_type<uint_t<B>>::type;

// int<B>
template <unsigned B>
requires ValidIntBits<B>
inline auto decode(int_t<B>, bytes_view data, size_t& pos) -> typename cpp_type<int_t<B>>::type;

// bool
inline auto decode(bool_t, bytes_view data, size_t& pos) -> typename cpp_type<bool_t>::type;

// address
inline auto decode(address_t, bytes_view data, size_t& pos) -> cpp_type<address_t>::type;

// bytes<N>
template<unsigned N>
requires ValidBytesN<N>
inline auto decode(bytes_fixed_t<N>, bytes_view data, size_t& pos) -> typename cpp_type<bytes_fixed_t<N>>::type;

// bytes
inline auto decode(bytes_dyn_t, bytes_view data, size_t& pos) -> cpp_type<bytes_dyn_t>::type;

// string
inline auto decode(string_t, bytes_view data, size_t& pos) -> cpp_type<string_t>::type;

// User-defined C++ type with `abi_tag`
template<HasAbiTupleTag T>
inline auto decode(T, bytes_view data, size_t& pos) -> typename cpp_type<T>::type;

// <T>[N]: fixed-length array of elements of the given type
template<class T, size_t N>
inline auto decode(array_t<T, N>, bytes_view data, size_t& pos) -> typename cpp_type<array_t<T, N>>::type;

// <T>[]: variable-length array of elements of the given type
template<class T>
inline auto decode(dyn_array_t<T>, bytes_view data, size_t& pos) -> typename cpp_type<dyn_array_t<T>>::type;

// (T1, T2,...,Tn): tuple consisting of the types T1, ..., Tn, n >= 0
template<class... Ts>
inline auto decode(tuple_t<Ts...>, bytes_view data, size_t& pos) -> typename cpp_type<tuple_t<Ts...>>::type;

// Helper function for decoding from a specific offset without modifying the global `pos`
// Useful when temporary or local decoding is needed without advancing the main cursor
template<AbiTag Tag>
inline auto decode_at(Tag tag, bytes_view data, size_t at) -> typename cpp_type<Tag>::type;

} // namespace internal

template<AbiTag Tag>
inline auto decode(bytes_view data, size_t& pos) -> typename cpp_type<Tag>::type;

template<AbiTag Tag>
inline auto decode(bytes_view data) -> typename cpp_type<Tag>::type;

template<HasAbiTupleTag T>
inline auto from_tuple(typename ::solabi::cpp_type<typename T::abi_tag>::type tup) -> T;

} // namespace solabi
