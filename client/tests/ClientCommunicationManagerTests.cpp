#include "gtest/gtest.h"
#include "CommunicationManager/ClientCommunicationManager.h"

class ClientCommunicationManagerFixture : public ::testing::Test {
protected:
    ClientCommunicationManager *communicationManager;

    void SetUp() override {
        communicationManager = new ClientCommunicationManager();
    }

    void TearDown() override {
        delete communicationManager;
    }
};
