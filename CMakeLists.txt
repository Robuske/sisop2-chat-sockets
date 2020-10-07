cmake_minimum_required(VERSION 3.17)
project(sisop2_t1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

# Tests were added following this tutorial https://www.jetbrains.com/help/clion/unit-testing-tutorial.html
# Google Test config
add_subdirectory(googletest-libs)

add_subdirectory(shared)

add_subdirectory(client)
add_subdirectory(server)
