//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_DEFAULT_FUNCTIONS_H
#define CVOLT_DEFAULT_FUNCTIONS_H

#include <iostream>

extern "C" inline void Out(int Num)
{
    std::cout << Num << std::endl;
}

extern "C" inline void OutStr(const char* Str)
{
    std::cout << Str << std::endl;
}

#endif //CVOLT_DEFAULT_FUNCTIONS_H