#include "ServerPersistency.h"
#include <iostream>
#include <fstream>

string ServerPersistency::getMessagesDatabasePathForGroup(const string &groupName) {
    // Building the file path based on the group name + file extension
    string path = groupName;
    string fileExtension = ".txt";
    path.append(fileExtension);

    return path;
}

/**
 * Function: ServerPersistency::saveMessage
 * Write 1 message on te related group file
 * @param[in] message
 * @param[out] operationCode
 */

int ServerPersistency::saveMessage(Message message) {
   string path = getMessagesDatabasePathForGroup(message.groupName);
    // Opening file in append mode
    std::ofstream file(path.c_str(), std::ios::app);
    Packet packet = message.asPacket();
    const char* data = (char*)&packet;
    file.write(data, sizeof(Packet));
    file.close();
    return CODE_SUCCESS;
}

/**
 * Function: ServerPersistency::readMessage
 * Read N messages (given in the server initialization) from the related group file
 * First we check if the file size is smaller than the number of bytes to be read.
 * In this case we read the number of bytes available in the file
 * Otherwise we use the number of bytes calculated using the provided messages count
 * @param[in] group, messageCount, messages
 * @param[out] numberOfMessagesRead
 */

// TODO: Pegar as ULTIMAS mensagens
int ServerPersistency::readMessages(string group, int messageCount, std::list<Message>& messages) {

    string path = getMessagesDatabasePathForGroup(group);
    std::ifstream file(path.c_str());

    // Checking the file size to avoid reading errors
    // To do so we need to set the file pointer to the EOF
    // Encapsulate in a different function (?)
    const auto begin = file.tellg();
    file.seekg (0, std::ios::end);
    const auto end = file.tellg();
    const long long fileSize = end-begin;

    //Rewinding file the file pointer previously located at the EOF
    file.seekg(0, std::ios::beg);

    const long long messagesSize = sizeof(Packet) * messageCount;

    const auto bytesToRead = std::min(fileSize, messagesSize);

    // Moving file pointer to the offset of the last massages to be read
    if(bytesToRead < fileSize) {
        int lastMessagesOffset = fileSize - messagesSize;
        file.seekg(lastMessagesOffset, std::ios::beg);
    }

    char *buffer = new char[bytesToRead]();

    file.read(buffer, bytesToRead);

    file.close();

    const int numberOfMessagesRead = bytesToRead/sizeof(Packet);

    for (int i = 0; i < numberOfMessagesRead; ++i) {
        Packet *packet = (Packet *) buffer;
        buffer+= sizeof(Packet);
        Message message = Message(*packet);
        messages.push_back(message);
    }

    return numberOfMessagesRead;
}


