#include <iostream>
#include "CommunicationManager/ServerCommunicationManager.h"

int main() {
    std::cout << "Hello, I am the Server" << std::endl;
    // Just to do something for now
    return ServerCommunicationManager().startServer(0);
}
