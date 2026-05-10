cmake_minimum_required(VERSION 3.20)

project(abi_decoder LANGUAGES CXX)

option(ABI_DECODER_BUILD_TESTS "Build abi-decoder tests" ON)

set(INTX_INSTALL OFF CACHE BOOL "" FORCE)
set(INTX_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/intx)

add_library(abi_decoder INTERFACE)
add_library(abi_decoder::abi_decoder ALIAS abi_decoder)

target_compile_features(abi_decoder INTERFACE cxx_std_20)
target_include_directories(abi_decoder INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(abi_decoder INTERFACE intx::intx)

if(ABI_DECODER_BUILD_TESTS)
    enable_testing()

    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    add_subdirectory(third_party/googletest)

    add_executable(abi_decoder_tests
        tests/abi_decode_composites_test.cpp
        tests/abi_decode_primitives_test.cpp
        tests/type_traits_test.cpp
        tests/utils_test.cpp
    )
    target_link_libraries(abi_decoder_tests
        PRIVATE
            abi_decoder::abi_decoder
            GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(abi_decoder_tests)
endif()
