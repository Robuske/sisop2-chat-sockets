#ifndef SISOP2_T1_SHAREDDEFINITIONS_H
#define SISOP2_T1_SHAREDDEFINITIONS_H

#include <string>
#include <iostream>

using std::string;

// TODO:
//  The PORT is here so we can share with client and don't to change the PORT every time we run the server.
//  Sometimes the port hangs and the server won't start until the OS close the release the resource
//  This should live in ServerDefinitions.h
#define PORT 2000

// Codes

#define CODE_SUCCESS 0

typedef int SocketFD;

enum PacketHeaderType { TypeConnection, TypeMessage };

struct tUserInfo {
    string username;
    string groupName;
} typedef UserInfo;

struct PacketHeader {
    PacketHeaderType type;
    uint16_t length;
};

struct Message {
    uint16_t timestamp;
    string group;
    string username;
    string text;
};

struct Packet {
    Message payload;
};


#endif // SISOP2_T1_SHAREDDEFINITIONS_H
