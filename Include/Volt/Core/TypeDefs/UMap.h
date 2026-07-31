//
// Created by bohdan on 06.05.26.
//

#ifndef CVOLT_UMAP_H
#define CVOLT_UMAP_H

#include <unordered_map>
#include "Volt/Core/Functions/FunctionSignature.h"
#include "Volt/Core/Hash/Hash.h"
#include "Volt/Compiler/Value/IRValue.h"
#include "Volt/Core/Functions/FunctionCallee.h"
#include "Volt/Core/Functions/FunctionOverload.h"

namespace Volt
{
    template <typename Key, typename Value>
    using UMap = std::unordered_map<Key, Value, Hash<Key>>;
    using FuncOverloadVector = SmallVec8<FunctionOverload>;
    using FunctionTable = llvm::StringMap<FuncOverloadVector>;
}

#endif //CVOLT_UMAP_H
