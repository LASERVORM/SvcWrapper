
# Application library
add_library(LibEchoServer SHARED
    include/EchoServer/echoserver_main.h

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

# Application executable
add_executable(EchoServer
    main.cpp
)

target_link_libraries(EchoServer
    PUBLIC
    LibEchoServer
)

# Application service
add_executable(EchoServerService
    service.cpp
)

target_link_libraries(EchoServerService
    PUBLIC
    LibEchoServer
    SvcWrapper
)
