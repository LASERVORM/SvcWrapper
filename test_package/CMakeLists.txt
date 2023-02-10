################################################################################
# Copyright (c) LASERVORM GmbH 2023                                            #
# CMake project for SvcWrapper conan test package.                             #
################################################################################

cmake_minimum_required(VERSION 3.19)
project(PackageTest CXX)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
find_package(SvcWrapper REQUIRED)

add_executable(test_package test_package.cpp)
target_link_libraries(test_package SvcWrapper::SvcWrapper)

enable_testing()
add_test(NAME test_package COMMAND test_package help)
