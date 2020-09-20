#include "gtest/gtest.h"
#include "definitions.h"
#include "MessagesManager/ServerMessagesManager.h"

class ServerMessagesManagerFixture : public ::testing::Test {
protected:
    ServerMessagesManager *messagesManager;

    void SetUp() override {
        messagesManager = new ServerMessagesManager();
    }

    void TearDown() override {
        delete messagesManager;
    }
};