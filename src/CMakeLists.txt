################################################################################
# CMake project for the SvcWrapper library                                     #
# Copyright (c) LASERVORM GmbH 2023                                            #
################################################################################

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Add SvcWrapper static library
add_library(SvcWrapper STATIC
    # Public headers
    ${PROJECT_SOURCE_DIR}/include/SvcWrapper/svcwrapper.h

    # Sources
    svcwrapper_impl.h
    svcwrapper_impl.cpp
    svccli.h
    svccli.cpp
)

target_include_directories(SvcWrapper
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
