project(Server_tests)

# Google Test config
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(server_tests
        ServerPersistencyTests.cpp ServerMessagesManagerTests.cpp ServerCommunicationManagerTests.cpp ServerGroupsManagerTests.cpp)

target_link_libraries(server_tests server_lib)
# Google Test config
target_link_libraries(server_tests gtest gtest_main)