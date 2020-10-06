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

int ServerPersistency::readMessage(string group, Message* messages, int messageCount) {
    string path = group;
    string fileExtension = ".txt";
    path.append(fileExtension);
    std::ifstream file(path.c_str()); //open in constructor

    char readBuffer[sizeof(Message) * messageCount];
    file.read(readBuffer, sizeof(Message));
    messages = (Message*) readBuffer;
    file.close();
    return 1;
}


