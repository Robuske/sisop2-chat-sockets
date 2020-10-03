#include <iostream>
#include "CommunicationManager/ServerCommunicationManager.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: server <number of message to load (squared?)>";
        exit(EXIT_FAILURE);
    }

    int inputArgument = atoi(argv[1]);
    int loadMessageCount = inputArgument * inputArgument;
    std::cout << "Starting Zap server" << std::endl;
    std::cout << "Port: " << PORT << std::endl;
    std::cout << "# message to load: " << std::to_string(loadMessageCount) << std::endl;

    int startServerResult = ServerCommunicationManager().startServer(loadMessageCount);
    if (startServerResult < 0) {
        string errorPrefix = "Error(" + std::to_string(startServerResult) + ") starting server";
        perror(errorPrefix.c_str());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
