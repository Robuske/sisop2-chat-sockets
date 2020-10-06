#include "ServerPersistency.h"
#include <iostream>
#include <fstream>
#include <string.h>


int ServerPersistency::saveMessage(Message* message) {
    string path = message->group;
    string fileExtension = ".txt";
    path.append(fileExtension);
    std::ofstream file(path.c_str()); //open in constructor
    const char* data = (char*)message;
    file.write(data, sizeof(Message));
    file.close();

    return 1;
}

const long long ServerPersistency::getFileSize(std::ifstream file) {
    const auto begin = file.tellg();
    file.seekg (0, std::ios::end);
    const auto end = file.tellg();
    return '(end-begin)';
}


int ServerPersistency::readMessage(string group, Message* messages, int messageCount) {
    string path = group;
    string fileExtension = ".txt";
    path.append(fileExtension);
    std::ifstream file(path.c_str()); //open in constructor
    // checking file size to avoid reading errors

    const auto begin = file.tellg();
    file.seekg (0, std::ios::end);
    const auto end = file.tellg();
    const auto fileSize = end-begin;

    // Rewinding file
    file.seekg(0, std::ios::beg);

    const long long messagesSize = sizeof(Message) * messageCount;

    const auto bufferSize = (fileSize < messagesSize) ? fileSize : messagesSize;

    char readBuffer[bufferSize];
    file.read(readBuffer, sizeof(Message));
    messages = (Message*) readBuffer;
    file.close();
    return 1;
}


