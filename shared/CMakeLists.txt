project(Shared)

include_directories(sources)

# Config shared_lib

add_library(shared_lib
        sources/SharedDefinitions.cpp sources/SharedDefinitions.h
        sources/Message/Message.cpp sources/Message/Message.h)

# Config Tests

add_subdirectory(tests)
