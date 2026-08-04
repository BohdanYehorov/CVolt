//
// Created by bohdan on 06.05.26.
//

#ifndef CVOLT_FUNCTIONDEFS_H
#define CVOLT_FUNCTIONDEFS_H

#include "Volt/Core/Functions/FunctionOverload.h"
#include "TypeDefs.h"
#include <llvm/ADT/StringMap.h>

namespace Volt
{
    using FuncOverloadVector = SmallVec8<FunctionOverload>;
    using FunctionMap = llvm::StringMap<FuncOverloadVector>;
}

#endif //CVOLT_FUNCTIONDEFS_H
