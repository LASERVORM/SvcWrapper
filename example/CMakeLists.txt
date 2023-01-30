################################################################################
# CMake project for the SvcWrapper library                                     #
# Copyright (c) LASERVORM GmbH 2023                                            #
################################################################################

# SvcWrapper demo application
#
# This small demo application implements a TCP echo server listening on
# 127.0.0.1:12345. It's used just to demonstrate that an application that works
# standanlone also works fine when wrapped by SvcWrapper.

set(CMAKE_AUTOMOC ON)
find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

# Shared example application
add_library(LibEchoServer SHARED
    # Public includes
    include/EchoServer/echoserver_main.h

    # Sources
    echoserver_main.cpp
    echoserver.h
    echoserver.cpp
)
target_include_directories(LibEchoServer
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(LibEchoServer
    PUBLIC
    Qt::Core
    Qt::Network
)

# Standalone application executable
add_executable(EchoServer
    main.cpp
)
target_link_libraries(EchoServer
    PUBLIC
    LibEchoServer
)

# Application service executable
add_executable(EchoServerService
    service.cpp
)
target_link_libraries(EchoServerService
    PUBLIC
    LibEchoServer
    SvcWrapper
)
