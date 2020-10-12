#include "gtest/gtest.h"
#include "Message/Message.h"

class MessageFixture : public ::testing::Test {
protected:
    Message *message;

    void SetUp() override {
//        message = new Message();
    }

    void TearDown() override {
//        delete message;
    }
};
