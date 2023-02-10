################################################################################
# CMake project for the SvcWrapper library                                     #
# Copyright (c) LASERVORM GmbH 2023                                            #
################################################################################

cmake_minimum_required(VERSION 3.19)

### Project metadata ###########################################################

set(LIB_VERSION_MAJOR 1)
set(LIB_VERSION_MINOR 0)
set(LIB_VERSION_PATCH 0)
project(SvcWrapper
    VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR}.${LIB_VERSION_PATCH}
    DESCRIPTION "Library for wrapping any C/C++ application into a Windows service."
    LANGUAGES CXX)

### Configurable options #######################################################

option(SVCWRAPPER_EXAMPLE "Build example application" OFF)

### Build options ##############################################################

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### Build targets ##############################################################

# SvcWrapper static libraray
add_subdirectory(src)

# Example application
if(SVCWRAPPER_EXAMPLE)
    add_subdirectory(example)
endif()

### Install rules ##############################################################

# Use a standard directory structure for install
include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKEDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

# Install header files
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)

# Generate package config
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)
write_basic_package_version_file(cmake/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)
install(FILES
    ${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake
    ${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

# Install export target definitions
install(EXPORT ${PROJECT_NAME}_exports
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
    NAMESPACE ${PROJECT_NAME}::
)
