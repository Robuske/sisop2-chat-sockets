#ifndef SISOP2_T1_MESSAGE_H
#define SISOP2_T1_MESSAGE_H

#include "SharedDefinitions.h"
#include <string>

using std::string;


class Message {
public:
    PacketType packetType;
    std::time_t timestamp;
    string groupName;
    string username;
    string text;

    // Constructors
    Message(PacketType packetType, std::time_t timestamp, string groupName, string username, string text);
    explicit Message(Packet packet);
    static Message keepAliveWithUsername(string username);

    const Packet asPacket();
};

#endif //SISOP2_T1_MESSAGE_H
