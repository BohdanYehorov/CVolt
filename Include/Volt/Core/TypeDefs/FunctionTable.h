//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_FUNCTIONTABLE_H
#define CVOLT_FUNCTIONTABLE_H

#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Compiler/Hash/FunctionSignatureHash.h"
#include "Volt/Compiler/Types/TypedValue.h"
#include "Volt/Core/Functions/FunctionCallee.h"

namespace Volt
{
	using FunctionTable = std::unordered_map<FunctionSignature, FunctionCallee*, FunctionSignatureHash>;
}

#endif //CVOLT_FUNCTIONTABLE_H