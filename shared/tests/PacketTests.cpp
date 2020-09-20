#include "gtest/gtest.h"
#include "Packet/Packet.h"

// Probably won't make sense to test this, created just to have something for now
class PacketFixture : public ::testing::Test {
protected:
    Packet *packet;

    void SetUp() override {
        packet = new Packet();
    }

    void TearDown() override {
        delete packet;
    }
};
