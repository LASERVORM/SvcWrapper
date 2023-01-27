
add_library(SvcWrapper STATIC
    ${PROJECT_SOURCE_DIR}/include/SvcWrapper/svcwrapper.h

    svcwrapper_impl.h
    svcwrapper_impl.cpp
)

target_include_directories(SvcWrapper
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
