################################################################################
# CMake project for the SvcWrapper library                                     #
# Copyright (c) LASERVORM GmbH 2023                                            #
################################################################################

### Static SvcWrapper library ##################################################

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
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

### Install rules ##############################################################

install(TARGETS SvcWrapper
    EXPORT ${PROJECT_NAME}_exports
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
