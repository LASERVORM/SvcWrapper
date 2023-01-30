################################################################################
# CMake project for the SvcWrapper library                                     #
# Copyright (c) LASERVORM GmbH 2023                                            #
################################################################################

cmake_minimum_required(VERSION 3.14)

project(SvcWrapper LANGUAGES CXX)
option(SVCWRAPPER_EXAMPLE "Build example application" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

if(SVCWRAPPER_EXAMPLE)
    add_subdirectory(example)
endif()
