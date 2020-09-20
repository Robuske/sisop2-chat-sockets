#include "gtest/gtest.h"
#include "definitions.h"
#include "UI/ClientUI.h"

class ClientUIFixture : public ::testing::Test {
protected:
    ClientUI *clientUI;

    void SetUp() override {
        clientUI = new ClientUI();
    }

    void TearDown() override {
        delete clientUI;
    }
};

TEST_F(ClientUIFixture, StartingClient) {
    EXPECT_EQ(clientUI->startClient("", "", "", ""), CODE_SUCCESS);
}
