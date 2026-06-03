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

namespace Volt
{
    template <typename Key, typename Value>
    using UMap = std::unordered_map<Key, Value, Hash<Key>>;
    using FunctionTable = UMap<FunctionSignature, FunctionCallee*>;
}

#endif //CVOLT_UMAP_H
