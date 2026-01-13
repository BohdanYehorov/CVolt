//
// Created by bohdan on 13.01.26.
//

#include <Volt/Compiler/Hash/FunctionSignatureHash.h>
#include <Volt/Compiler/Hash/DataTypeHash.h>
#include <Volt/Compiler/Hash/Hash.h>

namespace Volt
{
    size_t FunctionSignatureHash::operator()(const FunctionSignature &FuncSign) const
    {
        size_t Seed =  std::hash<std::string>{}(FuncSign.Name);
        for (auto Param : FuncSign.Params)
            CombineHashes(Seed, DataTypeHash{}(Param));

        return Seed;
    }
}