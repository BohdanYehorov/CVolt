//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_VARIABLETABLE_H
#define CVOLT_VARIABLETABLE_H

#include "../Value/TypedValue.h"

namespace Volt
{
	using CTimeVariableTable = std::unordered_map<std::string, CTimeValue*>;
	using VariableTable = std::unordered_map<std::string, TypedValue*>;
}

#endif //CVOLT_VARIABLETABLE_H