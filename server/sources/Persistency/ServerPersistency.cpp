#include "ServerPersistency.h"
#include <iostream>
#include <fstream>

/**
 * Function: ServerPersistency::saveMessage
 * Write 1 message on te related group file
 * @param[in] message
 * @param[out] operationCode
 */

int ServerPersistency::saveMessage(const Message& message) {
   // Building the file path based on the group name + file extension
    string path = message.group;
    string fileExtension = ".txt";
    path.append(fileExtension);
    // Opening file in append mode
    std::ofstream file(path.c_str(), std::ios::app);
    const char* data = (char*)&message;
    file.write(data, sizeof(Message));
    file.close();
    return CODE_SUCCESS;
}

/**
 * Function: ServerPersistency::calculateFileSize
 * Calculate the size of a ifstream file
 * The file in question must be a reference (parameters only don't work)
 * Not working at the moment
 * @param[in] filePointer
 * @param[out] fileSize
 */

//long long ServerPersistency::calculateFileSize(std::ifstream* filePointer) {
//    const auto begin = filePointer->tellg();
//    filePointer->seekg (0, std::ios::end);
//    const auto end = filePointer->tellg();
//    return end-begin;
//}

/**
 * Function: ServerPersistency::readMessage
 * Read N messages (given in the server initialization) from the related group file
 * First we check if the file size is smaller than the number of bytes to be read.
 * In this case we read the number of bytes available in the file
 * Otherwise we use the number of bytes calculated using the provided messages count
 * @param[in] group, messageCount, messages
 * @param[out] numberOfMessagesReaded
 */

int ServerPersistency::readMessages(string group, int messageCount, Message* messages) {

    string path = group;
    string fileExtension = ".txt";
    path.append(fileExtension);
    std::ifstream file(path.c_str());

    // Checking the file size to avoid reading errors
    // To do so we need to set the file pointer to the EOF
    // Encapsulate in a different function (?)
    const auto begin = file.tellg();
    file.seekg (0, std::ios::end);
    const auto end = file.tellg();
    const auto fileSize = end-begin;

    //const auto fileSize = this->calculateFileSize(&file);
    //Rewinding file the file pointer previously located at the EOF
    file.seekg(0, std::ios::beg);

    const auto messagesSize = sizeof(Message) * messageCount;

    const auto bufferSize = (fileSize < messagesSize) ? fileSize : messagesSize;

    file.read((char*)messages, bufferSize);

    file.close();

    return bufferSize/sizeof(Message);
}


