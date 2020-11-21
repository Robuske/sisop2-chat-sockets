#ifndef SISOP2_T1_MESSAGE_H
#define SISOP2_T1_MESSAGE_H

#include "SharedDefinitions.h"
#include <string>

using std::string;

class Message {
public:
    PacketType packetType;
    std::time_t timestamp;
    Client sender;
    Client recipient;
    string groupName;
    string username;
    string text;

    // Constructors
    Message(PacketType packetType, std::time_t timestamp, Client sender, Client recipient, string groupName, string username, string text);
    explicit Message(Packet packet);
    static Message keepAliveWithUsername(string username, Client sender, Client recipient);

    Packet asPacket() const;
};

#endif //SISOP2_T1_MESSAGE_H
