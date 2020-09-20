#include <iostream>
#include "UI/ClientUI.h"

int main() {
    std::cout << "Hello, I am the Client" << std::endl;
    // Just to do something for now
    return ClientUI().startClient("", "", "", "");
}
