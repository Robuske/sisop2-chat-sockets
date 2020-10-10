#include "GroupsAccessControl.h"

/**
 * Function: ServerMessagesManager::lockAccessForGroup
 * Lock the corresponding group mutex blocking other threads from writing/reading the group files
 * @param[in] groupName
 * @param[out] void
 */

void GroupsAccessControl::lockAccessForGroup(const string &groupName) {
    this->accessControl[groupName].lock();
}

/**
 * Function: ServerMessagesManager::unlockAccessForGroup
 * Unlock the corresponding group mutex allowing other threads to write/read in/from the group files
 * @param[in] groupName
 * @param[out] void
 */

void GroupsAccessControl::unlockAccessForGroup(const string &groupName) {
    this->accessControl[groupName].unlock();
}
