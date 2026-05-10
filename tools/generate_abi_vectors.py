#!/usr/bin/env python3

# Copyright (c) 2026 Ivan Nazarov
# SPDX-License-Identifier: MIT

from __future__ import annotations

import textwrap

try:
    from eth_abi import encode
except ImportError as exc:
    raise SystemExit(
        "Missing eth_abi. Install it with: python3 -m pip install eth-abi"
    ) from exc


VECTORS = [
    ("kUint256_42", ["uint256"], [42]),
    ("kUint8Max", ["uint8"], [255]),
    ("kUint64Pattern", ["uint64"], [0x1122334455667788]),
    ("kInt256Minus1", ["int256"], [-1]),
    ("kInt8Minus5", ["int8"], [-5]),
    ("kBoolTrue", ["bool"], [True]),
    ("kBoolFalse", ["bool"], [False]),
    ("kAddress", ["address"], ["0x1111111111111111111111111111111111111111"]),
    ("kBytes3", ["bytes3"], [bytes.fromhex("abcdef")]),
    ("kBytes32", ["bytes32"], [bytes.fromhex("00" * 31 + "7f")]),
    ("kBytesDynamic", ["bytes"], [bytes.fromhex("12345678")]),
    ("kBytesDynamicEmpty", ["bytes"], [b""]),
    ("kStringHi", ["string"], ["hi"]),
    ("kStringEmpty", ["string"], [""]),
    ("kUint256Array3", ["uint256[3]"], [[1, 2, 3]]),
    ("kStringArray2", ["string[2]"], [["hi", "bye"]]),
    ("kUint256DynArray", ["uint256[]"], [[1, 2, 3]]),
    ("kTupleUint256String", ["(uint256,string)"], [(7, "hi")]),
    ("kStaticRecord", ["(uint256,bool)"], [(9, True)]),
    ("kStaticRecordArray2", ["(uint256,bool)[2]"], [[(1, True), (2, False)]]),
    ("kDynamicRecordArray2", ["(uint256,string)[]"], [[(1, "one"), (2, "two")]]),
    ("kDynamicRecordFixedArray2", ["(uint256,string)[2]"], [[(1, "one"), (2, "two")]]),
    ("kStringDynArray", ["string[]"], [["hi", "bye"]]),
    ("kUint256PairDynArray", ["uint256[2][]"], [[[1, 2], [3, 4]]]),
    ("kUint256DynArrayFixedArray2", ["uint256[][2]"], [[[1, 2], [3]]]),
    ("kNestedStaticRecord", ["((uint256,bool),uint256)"], [((9, True), 11)]),
    (
        "kNestedDynamicRecord",
        ["((uint256,bool),(uint256,string),string)"],
        [((9, True), (7, "inner"), "outer")],
    ),
    (
        "kDeepRecord",
        ["(((uint256,bool),(uint256,string),string),(uint256,bool)[])"],
        [(((9, True), (7, "inner"), "outer"), [(1, True), (2, False)])],
    ),
    (
        "kNestedDynamicRecordArray2",
        ["((uint256,bool),(uint256,string),string)[]"],
        [
            [
                ((1, True), (10, "alpha"), "first"),
                ((2, False), (20, "beta"), "second"),
            ]
        ],
    ),
]


def print_vector(name: str, abi_types: list[str], values: list[object]) -> None:
    hex_data = encode(abi_types, values).hex()
    chunks = textwrap.wrap(hex_data, 64)

    print(f"inline constexpr std::string_view {name} =")
    for index, chunk in enumerate(chunks):
        suffix = ";" if index == len(chunks) - 1 else ""
        print(f'    "{chunk}"{suffix}')
    print()


def main() -> None:
    print("#pragma once")
    print()
    print("// clang-format off")
    print()
    print("#include <string_view>")
    print()
    print("namespace abi_test_vectors")
    print("{")
    for name, abi_types, values in VECTORS:
        print_vector(name, abi_types, values)
    print("} // namespace abi_test_vectors")
    print()
    print("// clang-format on")


if __name__ == "__main__":
    main()
