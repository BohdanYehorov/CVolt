//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_DEFAULT_FUNCTIONS_H
#define CVOLT_DEFAULT_FUNCTIONS_H

#include <iostream>
#include <cmath>

extern "C"
{
    inline void OutBool(bool B)
    {
        std::cout << std::boolalpha << B << std::endl;
    }

    inline void OutChar(char Ch)
    {
        std::cout << Ch << std::endl;
    }

    inline void OutByte(std::byte Byte)
    {
        std::cout << static_cast<int>(Byte) << std::endl;
    }

    inline void OutInt(int Num)
    {
        std::cout << Num << std::endl;
    }

    inline void OutLong(long Num)
    {
        std::cout << Num << std::endl;
    }

    inline void OutStr(const char* Str)
    {
        std::cout << Str << std::endl;
    }

    inline void OutFloat(float Num)
    {
        std::cout << Num << std::endl;
    }

    inline void OutDouble(double Num)
    {
        std::cout << Num << std::endl;
    }

    inline long Time()
    {
        return std::time(nullptr);
    }

    inline double Sin(double Angle)
    {
        return std::sin(Angle);
    }

    inline double Cos(double Andle)
    {
        return std::cos(Andle);
    }

    inline double Tan(double Angle)
    {
        return std::tan(Angle);
    }
}

#endif //CVOLT_DEFAULT_FUNCTIONS_H