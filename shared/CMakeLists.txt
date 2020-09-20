project(Shared)

include_directories(sources)

# Config shared_lib

add_library(shared_lib
        sources/SharedDefinitions.h
        sources/Packet/Packet.cpp sources/Packet/Packet.h)

# Config Tests

add_subdirectory(tests)
