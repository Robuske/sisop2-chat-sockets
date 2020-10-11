#ifndef SISOP2_T1_MESSAGE_H
#define SISOP2_T1_MESSAGE_H

#include "SharedDefinitions.h"
#include <string>

using std::string;

class Message {
public:
    Message(PacketType packetType);

    PacketType packetType;
    long timestamp;
    string groupName;
    string username;
    string text;

    Message(PacketType packetType, long timestamp, string groupName, string username, string text);
    explicit Message(Packet packet);

    const Packet asPacket();
};

#endif //SISOP2_T1_MESSAGE_H
