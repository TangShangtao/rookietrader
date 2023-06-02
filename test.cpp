#include "tools/Logger.h"
#include <iostream>
#include <string>

#define print std::cout << 
#define endl << std::endl



int main()
{
    int i = 0;
    const char* str = "helloworld";
    Logger::init();
    Logger::debug("debug{}", i);

}