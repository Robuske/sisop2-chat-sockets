#include <iostream>
#include "CommunicationManager/ServerCommunicationManager.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: server <number of message to load (squared?)>";
    }

    int loadMessageCount = atoi(argv[1]);
    std::cout << "Starting server with " << std::to_string(loadMessageCount) << " messages " << std::endl;

    return ServerCommunicationManager().startServer(loadMessageCount);
}
