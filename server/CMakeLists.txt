project(Server)

include_directories(sources)

add_library(server_lib
        sources/definitions.h
        sources/Persistency/ServerPersistency.cpp sources/Persistency/ServerPersistency.h
        sources/MessagesManager/ServerMessagesManager.cpp sources/MessagesManager/ServerMessagesManager.h
        sources/GroupsManager/ServerGroupsManager.cpp sources/GroupsManager/ServerGroupsManager.h
        sources/CommunicationManager/ServerCommunicationManager.cpp sources/CommunicationManager/ServerCommunicationManager.h)

add_executable(server sources/main.cpp)
target_link_libraries(server server_lib)

add_subdirectory(tests)