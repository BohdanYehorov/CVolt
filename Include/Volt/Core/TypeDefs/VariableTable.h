//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_VARIABLETABLE_H
#define CVOLT_VARIABLETABLE_H

#include "Volt/Core/Value/IRValue.h"

namespace Volt
{
	using CTimeVariableTable = std::unordered_map<std::string, ExprResult*>;
	using VariableTable = std::unordered_map<std::string, IRValue*>;
}

#endif //CVOLT_VARIABLETABLE_H