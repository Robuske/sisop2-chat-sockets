#include "gtest/gtest.h"
#include "definitions.h"
#include "GroupsManager/ServerGroupsManager.h"

class ServerGroupsManagerFixture : public ::testing::Test {
protected:
    ServerGroupsManager *groupsManager;

    void SetUp() override {
        groupsManager = new ServerGroupsManager();
    }

    void TearDown() override {
        delete groupsManager;
    }
};