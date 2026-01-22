cmake_minimum_required(VERSION 3.23)
project(Example LANGUAGES CXX)

message(STATUS
    "Using C++ Compiler: ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    #add_compile_options(--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/14)
endif()

add_executable(inproc-example)
target_sources(inproc-example
    PRIVATE
        src/inproc-example.cpp
        src/print_pkg_info.cpp
        src/test.cpp
    PRIVATE
        FILE_SET  HEADERS
        BASE_DIRS include
)
target_link_libraries(inproc-example
    PRIVATE
        Perfetto::perfetto
)

add_executable(sys-example)
target_sources(sys-example
    PRIVATE
        src/sys-example.cpp
        src/print_pkg_info.cpp
        src/test.cpp
    PRIVATE
        FILE_SET  HEADERS
        BASE_DIRS include
)
target_link_libraries(sys-example
    PRIVATE
        Perfetto::perfetto
)

if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    add_compile_options(
        -Wall -Wextra -Wpedantic
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        find_package(stdc++)
        if(stdc++_FOUND)
            add_library(stdc++)
        endif()
    endif()
endif()

find_package(Perfetto CONFIG REQUIRED)
