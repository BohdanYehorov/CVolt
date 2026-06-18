//
// Created by bohdan on 14.01.26.
//

#include <Volt/Tests/Fuzzer/ParserFuzzer.h>
#include <thread>
#include <llvm/ADT/StringSet.h>

namespace Volt
{
    std::mt19937 ParserFuzzer::Gen{ std::random_device{}() };

    std::string ParserFuzzer::RandomIntegerType()
    {
        bool IsSigned = RandomInt<int>(0, 1) == 1;
        return (IsSigned ? "i" : "u") + std::to_string((1 << RandomInt(0, 3)) * 8);
    }

    std::string ParserFuzzer::RandomFloatingPointType()
    {
        return "f" + std::to_string((1 << RandomInt(1, 4)) * 8);
    }
}
