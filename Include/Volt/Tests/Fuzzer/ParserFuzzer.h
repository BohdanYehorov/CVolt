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
        std::ofstream OutFile;

        size_t MaxDepth;

        bool InBlock = false;
        bool InFunction = false;
        bool InStatement = false;
        std::string Tabs;
        size_t TabsCount = 0;

    public:
        ParserFuzzer(size_t MaxDepth)
            : MaxDepth(MaxDepth) {}

        std::string GenExpr();
        std::string GenBlock();
        std::string GenFunctionDeclExpr();
        std::string GenVarDeclExpr();
        std::string GenIf();
        std::string GenWhile();
        std::string GenFor();
        std::string GenGlobalExpr();
        void StartTest(size_t Tests);

        std::string GenBinaryExpr();
        std::string GenPrefixUnary();
        std::string GenSuffixUnary();

        static std::string GenPrimary();
        static std::string GenIdentifier();
        static std::string GenByte();
        static std::string GenInt();
        static std::string GenLong();
        static std::string GenFloat();
        static std::string GenDouble();
    private:
        static Int8 RandomInt8(Int8 Min, Int8 Max);
        static int RandomInt(int Min, int Max);
        static Int64 RandomInt64(Int64 Min, Int64 Max);
        static float RandomFloat(float Min, float Max);
        static double RandomDouble(double Min, double Max);
    };
}

#endif //CVOLT_PARSERFUZZER_H