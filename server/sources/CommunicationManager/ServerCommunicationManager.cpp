#include "ServerCommunicationManager.h"
#include "SharedDefinitions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "iostream"

#define CODE_ERROR_WRITING_SOCKET -1

int ServerCommunicationManager::startServer(int loadMessageCount) {

    int initialSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (initialSocket == -1)
        std::cout << "Error opening socket" << errno << std::endl;

    int port = 2222;
    struct sockaddr_in serverAddress, clientAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serverAddress.sin_zero), 8);

    // Loop pq as vezes o SO nao libera a porta, dai tentamos varias...
    while (!bind(initialSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) {
        std::cout << "Error " << errno << " on binding to port " << port << std::endl;
        port++;
        serverAddress.sin_port = htons(port);
    }

    std::cout << "Servidor iniciado na porta " << port << std::endl;

    int maxNnumberOfConnections = 5;
    listen(initialSocket, maxNnumberOfConnections);

    socklen_t clilen = sizeof(struct sockaddr_in);
    int acceptedConnectionSocket = accept(initialSocket, (struct sockaddr *) &clientAddress, &clilen);
    if (acceptedConnectionSocket == -1)
        std::cout << "Error on accept: " << errno << std::endl;

    char buffer[256];
    bzero(buffer, 256);

    /* read from the socket */
    int bytesRead = read(acceptedConnectionSocket, buffer, 256);
    if (bytesRead < 0)
        std::cout << "Error reading from socket: " << errno << std::endl;
    std::cout << "Here is the message: " << buffer << std::endl;

    /* write in the socket */
    char letter = 'A';
    while (true) {
        if (letter == 'Z')
            letter = 'A';

        int bytesWritten = write(acceptedConnectionSocket, reinterpret_cast<const void *>(letter), 1);
        if (bytesWritten < 0) {
            std::cout << "Error writing to socket: " << errno;
            return CODE_ERROR_WRITING_SOCKET;
        }
        letter++;
    }

    close(acceptedConnectionSocket);
    close(initialSocket);

    return CODE_SUCCESS;
}
