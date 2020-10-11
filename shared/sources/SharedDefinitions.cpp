#include "SharedDefinitions.h"

std::time_t now() {
    std::time_t now = std::time(nullptr);
    return now;
}
