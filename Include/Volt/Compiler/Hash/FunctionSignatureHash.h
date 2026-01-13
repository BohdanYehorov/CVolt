//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_FUNCTIONSIGNATUREHASH_H
#define CVOLT_FUNCTIONSIGNATUREHASH_H
#include "Volt/Compiler/Functions/FunctionSignature.h"

namespace Volt
{
    class FunctionSignatureHash
    {
    public:
        size_t operator()(const FunctionSignature& FuncSign) const;
    };
}

#endif //CVOLT_FUNCTIONSIGNATUREHASH_H