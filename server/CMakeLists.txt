project(Server)

include_directories(sources)

# Config server_lib

add_library(server_lib
        sources/ServerDefinitions.h
        sources/Persistency/ServerPersistency.cpp sources/Persistency/ServerPersistency.h
        sources/MessagesManager/ServerMessagesManager.cpp sources/MessagesManager/ServerMessagesManager.h
        sources/GroupsManager/ServerGroupsManager.cpp sources/GroupsManager/ServerGroupsManager.h
        sources/CommunicationManager/ServerCommunicationManager.cpp sources/CommunicationManager/ServerCommunicationManager.h)

target_link_libraries(server_lib shared_lib)
target_include_directories(server_lib PUBLIC ./../shared/sources)

# Config server executable

add_executable(server sources/main.cpp)
target_link_libraries(server server_lib)

# Config tests

add_subdirectory(tests)