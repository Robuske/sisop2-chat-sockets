#include "Message.h"
#include <cstring>

Message::Message(PacketType packetType) {
    this->packetType = packetType;
    this->timestamp = 0;
    this->username = "";
    this->groupName = "";
    this->text = "A KEEP ALIVE MESSAGE";
}

Message::Message(PacketType packetType, long timestamp, string groupName, string username, string text) {
    this->packetType = packetType;
    this->timestamp = timestamp;
    this->username = username;
    this->groupName = groupName;
    this->text = text;
}

Message::Message(Packet packet) {
    this->packetType = packet.type;
    this->timestamp = packet.timestamp;
    this->username = std::string(packet.username);
    this->groupName = std::string(packet.groupName);
    this->text = std::string(packet.text);
}

const Packet Message::asPacket() {
    Packet packet{};

    packet.type = packetType;
    packet.timestamp = timestamp;
    strcpy(packet.username, username.c_str());
    strcpy(packet.groupName, groupName.c_str());
    strcpy(packet.text, text.c_str());

    return packet;
}
