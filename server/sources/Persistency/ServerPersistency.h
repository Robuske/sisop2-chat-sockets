#ifndef SISOP2_T1_SERVERPERSISTENCY_H
#define SISOP2_T1_SERVERPERSISTENCY_H

#include "SharedDefinitions.h"


class ServerPersistency {

public:
    int saveMessage(Message* message);
    int readMessage(string group, Message* messages, int messageCount);

    const long long int getFileSize(std::ifstream file);
};


#endif //SISOP2_T1_SERVERPERSISTENCY_H
