//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_DEFAULT_FUNCTIONS_H
#define CVOLT_DEFAULT_FUNCTIONS_H

#include <iostream>

extern "C"
{
    inline void OutInt(int Num)
    {
        std::cout << Num;
    }

    inline void OutStr(const char* Str)
    {
        std::cout << Str;
    }
}

#endif //CVOLT_DEFAULT_FUNCTIONS_H