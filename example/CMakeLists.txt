# Copyright (c) 2026 Ivan Nazarov
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.20)

project(solabi_example LANGUAGES CXX)

find_package(solabi CONFIG REQUIRED)

add_executable(solabi_example
    main.cpp
)

target_link_libraries(solabi_example
    PRIVATE
        solabi::solabi
)
