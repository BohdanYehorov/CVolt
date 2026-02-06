//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_FUNCTIONTABLE_H
#define CVOLT_FUNCTIONTABLE_H

#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Compiler/Types/TypedValue.h"

namespace Volt
{
	using FunctionTable = std::unordered_map<FunctionSignature, TypedFunction*, FunctionSignatureHash>;
}

#endif //CVOLT_FUNCTIONTABLE_H