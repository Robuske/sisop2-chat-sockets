project(Client)

include_directories(sources)

# Config client_lib

add_library(client_lib
        sources/ClientDefinitions.h
        sources/UI/ClientUI.cpp sources/UI/ClientUI.h
        sources/MessagesManager/ClientMessagesManager.cpp sources/MessagesManager/ClientMessagesManager.h
        sources/CommunicationManager/ClientCommunicationManager.cpp sources/CommunicationManager/ClientCommunicationManager.h)

target_link_libraries(client_lib shared_lib)
target_include_directories(client_lib PUBLIC ./../shared/sources)

# Config client executable

add_executable(client sources/main.cpp)
target_link_libraries(client client_lib)

# Config Tests

add_subdirectory(tests)