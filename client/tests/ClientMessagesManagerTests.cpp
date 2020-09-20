#include "gtest/gtest.h"
#include "MessagesManager/ClientMessagesManager.h"

class ClientMessagesManagerFixture : public ::testing::Test {
protected:
    ClientMessagesManager *messagesManager;

    void SetUp() override {
        messagesManager = new ClientMessagesManager();
    }

    void TearDown() override {
        delete messagesManager;
    }
};
