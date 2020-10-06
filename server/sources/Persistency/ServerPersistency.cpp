#include "ServerPersistency.h"
#include <iostream>
#include <fstream>

int ServerPersistency::saveMessage(Message* message) {
    std::ofstream groupFile;
    double writtenValue = 2.2;
    string groupName = message->group;
    groupName.append(".dat");
    string teste = "teste";
    groupFile.open(groupName, std::ios::binary | std::ios::out);
    groupFile.write(teste.c_str(), teste.length());
    return 1;
}

int ServerPersistency::readMessage(string group) {
    std::ofstream groupFile;
    double writtenValue = 2.2;
    string groupName = group;
    groupName.append(".dat");
    string teste = "teste";
    groupFile.open(groupName, std::ios::binary | std::ios::out);
    groupFile.write(teste.c_str(), teste.length());
    return 1;
}


