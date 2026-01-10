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

    inline void OutFloat(float Num)
    {
        std::cout << Num;
    }

    inline void OutLong(long Num)
    {
        std::cout << Num;
    }

    inline long Time()
    {
        return std::time(nullptr);
    }
}

#endif //CVOLT_DEFAULT_FUNCTIONS_H