
cmake_minimum_required(VERSION 2.6)
project(HW1)

ENABLE_TESTING()

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_BUILD_TYPE Release)

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -pg -pthread")
    ADD_DEFINITIONS(
        -Wall
        -Wextra
        -pedantic
        -Wno-long-long
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wredundant-decls
        -Wpointer-arith
        -Wcast-align
        -Wwrite-strings
#        -Winline
        -Wswitch-enum
        -Wswitch-default
        -std=c++0x
    )
endif ()

add_executable(client
    direction.cc
    main.cc
    search.cc
    maze.cc
    node.cc
    csocket.cc
    tile.cc
)

