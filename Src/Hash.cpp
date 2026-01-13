//
// Created by bohdan on 08.01.26.
//

#include "Volt/Compiler/Hash/Hash.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"

namespace Volt
{
    size_t CombineHashes(llvm::ArrayRef<size_t> Hashes)
    {
        size_t Res = 0;

        for (size_t Hash : Hashes)
            Res ^= Hash + 0x9e3779b9 + (Res << 6) + (Res >> 2);

        return Res;
    }
}