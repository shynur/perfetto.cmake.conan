cmake_minimum_required(VERSION 3.23)
project(Example LANGUAGES CXX)

message(STATUS
    "Using C++ Compiler: ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/14)
endif()

add_executable(example)
target_sources(example
    PRIVATE
        src/main.cpp
        src/print_pkg_info.cpp
    PRIVATE
        FILE_SET  HEADERS
        BASE_DIRS include
)
target_link_libraries(example
    PRIVATE
        QuickJS::perfetto
)
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    target_compile_options(example
        PRIVATE
            #-std=c++26
            -Wall
            -Wextra
            -Wpedantic
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_link_libraries(example
            PRIVATE
                stdc++
        )
    endif()
endif()

find_package(Perfetto CONFIG REQUIRED)
