#include <iostream>
#include "CommunicationManager/ServerCommunicationManager.h"

int main(int argc, char *argv[]) {
    std::cout << "Hello, I am the Server" << std::endl;

    if (argc > 1) {
        std::cout << "Usage: server loadMessageCount";
    }

    int loadMessageCount = atoi(argv[0]);
    std::cout << "Starting server with " << std::to_string(loadMessageCount) << " messages " << std::endl;

    return ServerCommunicationManager().startServer(loadMessageCount);
}
