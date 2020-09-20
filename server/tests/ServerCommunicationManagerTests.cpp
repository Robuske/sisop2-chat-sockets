#include "gtest/gtest.h"
#include "definitions.h"
#include "CommunicationManager/ServerCommunicationManager.h"

class ServerCommunicationManagerFixture : public ::testing::Test {
protected:
    ServerCommunicationManager *communicationManager;

    void SetUp() override {
        communicationManager = new ServerCommunicationManager();
    }

    void TearDown() override {
        delete communicationManager;
    }
};

TEST_F(ServerCommunicationManagerFixture, StartingClient) {
    EXPECT_EQ(communicationManager->startServer(0), CODE_SUCCESS);
}
