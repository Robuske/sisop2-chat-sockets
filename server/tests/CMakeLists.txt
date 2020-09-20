project(Server_Tests)

# Config for Google Test
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Config server_tests executable

add_executable(server_tests
        ServerPersistencyTests.cpp ServerMessagesManagerTests.cpp ServerCommunicationManagerTests.cpp ServerGroupsManagerTests.cpp)

target_link_libraries(server_tests server_lib)

# Config for Google Test
target_link_libraries(server_tests gtest gtest_main)