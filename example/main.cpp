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
    const auto encoded_number = solabi::from_hex(
        "000000000000000000000000000000000000000000000000000000000000002a");

    const auto encoded_string = solabi::from_hex(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "0000000000000000000000000000000000000000000000000000000000000005"
        "68656c6c6f000000000000000000000000000000000000000000000000000000");

    const auto encoded_message = solabi::from_hex(
        "0000000000000000000000000000000000000000000000000000000000000020"
        "000000000000000000000000000000000000000000000000000000000000002a"
        "0000000000000000000000000000000000000000000000000000000000000040"
        "0000000000000000000000000000000000000000000000000000000000000005"
        "68656c6c6f000000000000000000000000000000000000000000000000000000");

    const auto number = solabi::decode<solabi::uint_t<256>>(
        bytes_view{encoded_number.data(), encoded_number.size()});

    const auto text = solabi::decode<solabi::string_t>(
        bytes_view{encoded_string.data(), encoded_string.size()});

    const auto message = solabi::decode<Message>(
        bytes_view{encoded_message.data(), encoded_message.size()});

    std::cout << "number: " << intx::to_string(number) << '\n';
    std::cout << "text: " << text << '\n';
    std::cout << "message.number: " << intx::to_string(message.number) << '\n';
    std::cout << "message.text: " << message.text << '\n';
}
