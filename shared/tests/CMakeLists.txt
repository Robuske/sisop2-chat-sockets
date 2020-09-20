project(Shared_Tests)

# Config for Google Test
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Config shared_tests executable

add_executable(shared_tests
        PacketTests.cpp)

target_link_libraries(shared_tests shared_lib)

# Config for Google Test
target_link_libraries(shared_tests gtest gtest_main)
