# Copyright (c) 2026 Ivan Nazarov
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

project(abi_decoder VERSION 0.1.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(ABI_DECODER_IS_TOP_LEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(ABI_DECODER_IS_TOP_LEVEL ON)
endif()

option(ABI_DECODER_BUILD_TESTS "Build abi-decoder tests" ON)
option(ABI_DECODER_BUILD_EXAMPLES "Build abi-decoder examples" ON)
option(ABI_DECODER_INSTALL "Install abi-decoder package" ${ABI_DECODER_IS_TOP_LEVEL})

set(INTX_INSTALL ${ABI_DECODER_INSTALL} CACHE BOOL "" FORCE)
set(INTX_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/intx)

add_library(abi_decoder INTERFACE)
add_library(abi_decoder::abi_decoder ALIAS abi_decoder)
add_library(solabi::solabi ALIAS abi_decoder)

set_target_properties(abi_decoder PROPERTIES
    EXPORT_NAME solabi
)

target_compile_features(abi_decoder INTERFACE cxx_std_20)
target_include_directories(abi_decoder INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(abi_decoder INTERFACE intx::intx)

if(ABI_DECODER_INSTALL)
    set(ABI_DECODER_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/solabi")

    configure_package_config_file(
        cmake/solabiConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/solabiConfig.cmake"
        INSTALL_DESTINATION "${ABI_DECODER_INSTALL_CMAKEDIR}"
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/solabiConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(TARGETS abi_decoder
        EXPORT solabiTargets
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    install(EXPORT solabiTargets
        FILE solabiTargets.cmake
        NAMESPACE solabi::
        DESTINATION "${ABI_DECODER_INSTALL_CMAKEDIR}"
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/solabiConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/solabiConfigVersion.cmake"
        DESTINATION "${ABI_DECODER_INSTALL_CMAKEDIR}"
    )
endif()

if(ABI_DECODER_BUILD_EXAMPLES)
    add_executable(abi_decoder_example
        example/main.cpp
    )
    target_link_libraries(abi_decoder_example
        PRIVATE
            abi_decoder::abi_decoder
    )
endif()

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
