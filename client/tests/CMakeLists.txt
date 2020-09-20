project(Client_tests)

# Google Test config
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(client_tests
        ClientUITests.cpp ClientCommunicationManagerTests.cpp ClientMessagesManagerTests.cpp)

target_link_libraries(client_tests client_lib)
# Google Test config
target_link_libraries(client_tests gtest gtest_main)