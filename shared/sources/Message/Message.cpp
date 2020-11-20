#include "Message.h"
#include <cstring>

Message::Message(PacketType packetType, std::time_t timestamp, Client sender, Client recipient, string groupName,
                 string username, string text) {
    this->packetType = packetType;
    this->timestamp = timestamp;
    this->sender = sender;
    this->recipient = recipient;
    this->username = username;
    this->groupName = groupName;
    this->text = text;
}

Message::Message(Packet packet) {
    this->packetType = packet.type;
    this->timestamp = packet.timestamp;
    this->sender = packet.sender;
    this->recipient = packet.recipient;
    this->username = std::string(packet.username);
    this->groupName = std::string(packet.groupName);
    this->text = std::string(packet.text);
}

Message Message::keepAliveWithUsername(string username, Client sender, Client recipient) {
    return Message(TypeKeepAlive, now(), sender, recipient, "", username, "KEEP ALIVE MESSAGE");
}

Packet Message::asPacket() const {
    Packet packet{};

    packet.type = packetType;
    packet.timestamp = timestamp;
    packet.sender = sender;
    packet.recipient = recipient;
    strcpy(packet.username, username.c_str());
    strcpy(packet.groupName, groupName.c_str());
    strcpy(packet.text, text.c_str());

    return packet;
}
