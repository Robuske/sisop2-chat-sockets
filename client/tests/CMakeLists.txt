project(Client_Tests)

# Config for Google Test
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Config client_tests executable

add_executable(client_tests
        ClientUITests.cpp ClientCommunicationManagerTests.cpp ClientMessagesManagerTests.cpp)

target_link_libraries(client_tests client_lib)

# Config for Google Test
target_link_libraries(client_tests gtest gtest_main)