#include "SharedDefinitions.h"
#include "ClientUI.h"
#include "iostream"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define CODE_ERROR_READING_SOCKET -123

int ClientUI::startClient(string userName, string groupName, string serverIPAddress, string port) {
    std::cout << "Cliente iniciado" << std::endl;
    std::cout << userName << "@" << serverIPAddress << ":" << port << "/" << groupName << "/db.txt.chupa" << std::endl;

    struct hostent *server = gethostbyname(serverIPAddress.c_str());
    if (server == NULL) {
        std::cout << "Host " << serverIPAddress << " desconhecido" << std::endl;
        std::cout << stderr;
        exit(0);
    }

    int socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1)
        std::cout << "ERROR opening socket" << std::endl;

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(std::stoi(port));
    serverAddress.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serverAddress.sin_zero), 8);

    int connection = connect(socketFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if (connection < 0) {
        std::cout << "Error connecting: " << errno   << std::endl;
    }

    int bufferSize = 256;
    char buffer[bufferSize];
    printf("Mensagem >  ");
    bzero(buffer, bufferSize);
    fgets(buffer, bufferSize, stdin);

    /* write in the socket */
    int bytesWriten = write(socketFileDescriptor, buffer, bufferSize);
    if (bytesWriten < 0) {
        std::cout << "Error writing to socket: " << errno << std::endl;
    }
    bzero(buffer,256);

    /* read from the socket */
    while (true) {
        int bytesRead = read(socketFileDescriptor, buffer, 256);
        if (bytesRead < 0) {
            std::cout << "Error reading socket: " << errno;
            std::cout << stderr;
            return CODE_ERROR_READING_SOCKET;
        }
    }

    printf("%s\n",buffer);

    close(socketFileDescriptor);

    return CODE_SUCCESS;
}
