//
// Created by bohdan on 14.01.26.
//

#ifndef CVOLT_PARSERFUZZER_H
#define CVOLT_PARSERFUZZER_H
#include "Volt/Core/Parser/Parser.h"
#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include <string>
#include <random>
#include <fstream>

namespace Volt
{
    class ParserFuzzer
    {
    private:
        static std::mt19937 Gen;

        size_t MaxStatementsCount;
        size_t MaxDepth;

    public:
        ParserFuzzer()
            : MaxDepth(MaxDepth) {}

    //private:
        std::string RandomIntegerType();
        std::string RandomFloatingPointType();

    private:
        template <typename T>
        static T RandomInt(T Min, T Max)
        {
            std::uniform_int_distribution<T> Dist(Min, Max);
            return Dist(Gen);
        }

        template <typename T>
        static T RandomFloat(T Min, T Max)
        {
            std::uniform_real_distribution<T> Dist(Min, Max);
            return Dist(Gen);
        }
    };
}

#endif //CVOLT_PARSERFUZZER_H