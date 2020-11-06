project(Front)

include_directories(sources)

# Config front_lib

add_library(front_lib
        sources/FrontDefinitions.h
        sources/FrontCommunicationManager.cpp sources/FrontCommunicationManager.h)

target_link_libraries(front_lib shared_lib)
target_include_directories(front_lib PUBLIC ./../shared/sources)

# Config front executable

add_executable(front sources/main.cpp)
target_link_libraries(front front_lib)