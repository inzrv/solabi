# solabi

Header-only C++20 decoder for Solidity ABI values.

`solabi` maps Solidity ABI type tags to C++ carrier types and decodes ABI-encoded byte buffers into plain C++ values, tuples, arrays, vectors,and user-defined aggregate types.

## Example

```cpp
#include <solabi/decoder.h>
#include <solabi/utils.h>

#include <iostream>
#include <string>

struct Message
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::string_t>;

    intx::uint256 number;
    std::string text;
};

int main()
{
    const auto encoded = solabi::from_hex(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "000000000000000000000000000000000000000000000000000000000000002a"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000005"
        "68656c6c6f000000000000000000000000000000000000000000000000000000");

    const auto message = solabi::decode<Message>(bytes_view{encoded.data(), encoded.size()});

    std::cout << intx::to_string(message.number) << '\n';
    std::cout << message.text << '\n';
}
```

See [example/main.cpp](example/main.cpp) for a complete runnable example.

## Requirements

- C++20 compiler
- CMake 3.20+
- `intx` for 256-bit integer storage
- GoogleTest only when building tests

Dependencies are vendored as git submodules:

```bash
git submodule update --init --recursive
```

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the example:

```bash
./build/abi_decoder_example
```

Useful CMake options:

```bash
-DABI_DECODER_BUILD_TESTS=ON
-DABI_DECODER_BUILD_EXAMPLES=ON
-DABI_DECODER_INSTALL=ON
```

## CMake Usage

Vendored with `add_subdirectory`:

```cmake
add_subdirectory(third_party/abi-decoder)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE solabi::solabi)
```

Installed package with `find_package`:

```bash
cmake -S . -B build
cmake --build build
cmake --install build --prefix /path/to/prefix
```

Then from another project:

```cmake
find_package(solabi CONFIG REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE solabi::solabi)
```

Configure that project with:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/prefix
```

## Supported ABI Tags

| Solidity ABI type | `solabi` tag | C++ carrier |
| --- | --- | --- |
| `uint<M>` | `solabi::uint_t<M>` | `intx::uint256` |
| `int<M>` | `solabi::int_t<M>` | `intx::uint256` two's-complement bits |
| `bool` | `solabi::bool_t` | `bool` |
| `address` | `solabi::address_t` | `bytes` with 20 bytes |
| `bytes<M>` | `solabi::bytes_fixed_t<M>` | `bytes` |
| `bytes` | `solabi::bytes_dyn_t` | `bytes` |
| `string` | `solabi::string_t` | `std::string` |
| `T[M]` | `solabi::array_t<T, M>` | `std::array<cpp_type<T>, M>` |
| `T[]` | `solabi::dyn_array_t<T>` | `std::vector<cpp_type<T>>` |
| `(T1,T2,...)` | `solabi::tuple_t<T1, T2, ...>` | `std::tuple<cpp_type<Ts>...>` |

User-defined C++ types can be decoded when they expose a tuple ABI tag:

```cpp
struct User
{
    using abi_tag = solabi::tuple_t<solabi::uint_t<256>, solabi::bool_t>;

    intx::uint256 id;
    bool active;
};

auto user = solabi::decode<User>(bytes_view{data.data(), data.size()});
```

The user type must be constructible from the decoded tuple values, effectively:

```cpp
User{id, active}
```

## Utilities

`solabi::from_hex(std::string_view)` converts hex strings into `bytes`.
It accepts optional `0x` / `0X` prefixes and rejects malformed input.

## Current Scope

- Decoding only; ABI encoding is not implemented.
- Signed integers are returned as raw two's-complement bits in `intx::uint256`.
- Dynamic offsets, payload bounds, canonical bool/address encoding, integer width, and bytes/string padding are validated.
- Test vectors are generated from Python `eth_abi`:

```bash
python3 tools/generate_abi_vectors.py > tests/abi_vectors.h
```

## Formatting

The project uses `clang-format`:

```bash
clang-format -i include/solabi/*.h tests/*.cpp tests/*.h example/*.cpp
```

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE.txt) file for details.
