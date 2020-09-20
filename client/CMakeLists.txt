project(Client)

include_directories(sources)

add_library(client_lib
        sources/definitions.h
        sources/UI/ClientUI.cpp sources/UI/ClientUI.h
        sources/MessagesManager/ClientMessagesManager.cpp sources/MessagesManager/ClientMessagesManager.h
        sources/CommunicationManager/ClientCommunicationManager.cpp sources/CommunicationManager/ClientCommunicationManager.h)

add_executable(client sources/main.cpp)
target_link_libraries(client client_lib)

add_subdirectory(tests)