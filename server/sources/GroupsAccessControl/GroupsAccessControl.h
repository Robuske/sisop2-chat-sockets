#ifndef SISOP2_T1_GROUPSACCESSCONTROL_H
#define SISOP2_T1_GROUPSACCESSCONTROL_H

#include "SharedDefinitions.h"
#include <iostream>
#include <list>
#include <map>
#include <mutex>

/**
 * Map: SharedResourcesAccessControl
 * Responsible to manage the group users access to the group files
 * @attributes [groupName, groupMutex]
 */

typedef std::map<string , std::mutex> SharedResourcesAccessControl;

class GroupsAccessControl {

private:
    SharedResourcesAccessControl accessControl;

public:
    void lockAccessForGroup(const string& groupName);
    void unlockAccessForGroup(const string& groupName);
};

#endif //SISOP2_T1_GROUPSACCESSCONTROL_H
