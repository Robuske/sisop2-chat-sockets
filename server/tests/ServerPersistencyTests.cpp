#include "gtest/gtest.h"
#include "definitions.h"
#include "Persistency/ServerPersistency.h"

class ServerPersistencyFixture : public ::testing::Test {
protected:
    ServerPersistency *persistency;

    void SetUp() override {
        persistency = new ServerPersistency();
    }

    void TearDown() override {
        delete persistency;
    }
};
