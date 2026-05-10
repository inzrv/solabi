#pragma once

#include "decoder.h"

#include <string>
#include <utility>
#include <vector>

struct StaticRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>;

    intx::uint256 id;
    bool active;
};

struct DynamicRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::string_t>;

    intx::uint256 id;
    std::string label;
};

struct NestedStaticRecord
{
    using abi_tag = solabi::tuple_t<StaticRecord, solabi::uint_t<256>>;

    StaticRecord owner;
    intx::uint256 score;
};

struct NestedDynamicRecord
{
    using abi_tag = solabi::tuple_t<StaticRecord, DynamicRecord, solabi::string_t>;

    StaticRecord owner;
    DynamicRecord payload;
    std::string note;
};

struct DeepRecord
{
    using abi_tag = solabi::tuple_t<NestedDynamicRecord, solabi::dyn_array_t<StaticRecord>>;

    NestedDynamicRecord nested;
    std::vector<StaticRecord> history;
};

struct ConstructorRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::string_t>;

    ConstructorRecord() = default;
    ConstructorRecord(intx::uint256 id_, std::string label_)
        : id{id_}, label{std::move(label_)}
    {}

    intx::uint256 id;
    std::string label;
};

struct MissingAbiTag
{
    int value{};
};

struct InvalidAbiTag
{
    using abi_tag = int;

    int value{};
};

struct PrimitiveAbiTag
{
    using abi_tag = solabi::uint_t<256>;

    intx::uint256 value{};
};

struct WrongFieldCountRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>;

    intx::uint256 id;
};

struct NoDefaultRecord
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>;

    NoDefaultRecord(intx::uint256 id_, bool active_)
        : id{id_}, active{active_}
    {}

    intx::uint256 id;
    bool active;
};
