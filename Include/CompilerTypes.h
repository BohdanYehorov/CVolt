//
// Created by bohdan on 21.12.25.
//

#ifndef CVOLT_INSTRUCTION_H
#define CVOLT_INSTRUCTION_H

#include <string>
#include "TypedValue.h"

namespace Volt
{
    struct ScopeEntry
    {
        std::string Name;
        TypedValue* Previous = nullptr;
    };

    struct CompilerError : std::exception
    {
        std::string Str;
        CompilerError(std::string&& Str) : Str(std::move(Str)) {}
        [[nodiscard]] const char* what() const noexcept override { return Str.c_str(); }
    };
}

#endif //CVOLT_INSTRUCTION_H