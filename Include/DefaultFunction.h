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

#endif //CVOLT_DEFAULT_FUNCTIONS_H