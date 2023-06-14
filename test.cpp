#include <thread>
#include <iostream>
#include <string>
#include <stdint.h>
#include <string.h>
#include <fstream>
#define print std::cout << 
#define endl << std::endl



int main()
{
    std::string tests = "1234abcd";
    int pos = tests.find_last_not_of("cd");
    tests.erase(pos);
    print tests endl;
    
}