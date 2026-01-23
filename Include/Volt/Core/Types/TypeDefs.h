//
// Created by bohdan on 22.01.26.
//

#ifndef CVOLT_TYPEDEFS_H
#define CVOLT_TYPEDEFS_H

#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Compiler/Types/TypedValue.h"
#include "Volt/Compiler/Hash/FunctionSignatureHash.h"
#include <unordered_map>
#include <string>

namespace Volt
{
	using FunctionTable = std::unordered_map<FunctionSignature, TypedFunction*, FunctionSignatureHash>;
	using VariableTable = std::unordered_map<std::string, TypedValue*>;
}

#endif //CVOLT_TYPEDEFS_H