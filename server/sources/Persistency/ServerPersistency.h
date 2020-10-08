#ifndef SISOP2_T1_SERVERPERSISTENCY_H
#define SISOP2_T1_SERVERPERSISTENCY_H

#include "SharedDefinitions.h"

class ServerPersistency {

public:
    int saveMessage(const Message& message);
    int readMessages(string group, int messageCount, Message* messages);

private:
   // long long calculateFileSize(std::ifstream* filePointer);
};


#endif //SISOP2_T1_SERVERPERSISTENCY_H
