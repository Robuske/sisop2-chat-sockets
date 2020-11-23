#ifndef SISOP2_T1_SERVERDEFINITIONS_H
#define SISOP2_T1_SERVERDEFINITIONS_H

#define MAX_CONNECTIONS_COUNT 2
#define ELECTION_RUNNING -1

#define SUCCESSFUL_OPERATION 1

#define ERROR_SOCKET_BINDING -2
#define ERROR_SOCKET_ACCEPT_CONNECTION -228

#define ERROR_GROUP_NOT_FOUND -17
#define ERROR_MAX_USER_CONNECTIONS_REACHED -20
#define ERROR_FRONT_DISCONNECTED -135

#define ERROR_CONNECTING_SOCKET_SERVER_TO_SERVER -240

#define SERVER_REPLICAS_NUMBER 5

struct UserConnection {
    string username;
    SocketFD frontSocket;
    Client origin;

    bool operator==(const struct UserConnection& a) const {
        return ( a.frontSocket == this->frontSocket && a.username == this->username && a.origin == this->origin);
    }
};

#endif //SISOP2_T1_SERVERDEFINITIONS_H
