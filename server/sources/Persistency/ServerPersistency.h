#ifndef SISOP2_T1_SERVERPERSISTENCY_H
#define SISOP2_T1_SERVERPERSISTENCY_H

#include "Message/Message.h"
#include "SharedDefinitions.h"
#include <list>

class ServerPersistency {

public:
    int saveMessage(Message message);
    int readMessages(string group, int messageCount, std::list<Message>& messages);

private:
    string getMessagesDatabasePathForGroup(const string& groupName);
    // TODO: Essa função?
   // long long calculateFileSize(std::ifstream* filePointer);
};


#endif //SISOP2_T1_SERVERPERSISTENCY_H
