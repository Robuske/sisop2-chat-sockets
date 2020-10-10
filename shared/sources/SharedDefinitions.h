#ifndef SISOP2_T1_SHAREDDEFINITIONS_H
#define SISOP2_T1_SHAREDDEFINITIONS_H

#include <string>

using std::string;

// TODO:
//  The PORT is here so we can share with client and don't to change the PORT every time we run the server.
//  Sometimes the port hangs and the server won't start until the OS close the release the resource
//  This should live in ServerDefinitions.h
#define PORT 2000

// Codes

#define ERROR_SOCKET_CREATION -2
#define ERROR_SOCKET_WRITE -18
#define ERROR_SOCKET_READ -20
#define CODE_SUCCESS 0

#define NAME_SIZE 128
#define MESSAGE_SIZE 512

typedef int SocketFD;

enum PacketType { TypeConnection, TypeDesconnection, TypeMessage, TypeKeepAlive };

struct tUserInfo {
    string username;
    string groupName;
} typedef UserInfo;

//struct PacketHeader {
//    PacketHeaderType type;
//};

struct Packet {
    PacketType type;
    uint16_t timestamp;
    char groupName[NAME_SIZE];
    char username[NAME_SIZE];
    char text[MESSAGE_SIZE];
};

//struct Packet {
//    Message payload;
//};


#endif // SISOP2_T1_SHAREDDEFINITIONS_H
