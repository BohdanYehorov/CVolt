//
// Created by bohdan on 17.12.25.
//

#ifndef CVOLT_ERRORS_H
#define CVOLT_ERRORS_H

#include <vector>
#include <string>
#include <format>
#include <llvm/ADT/APFloat.h>

namespace Volt
{
    struct Error
    {
        size_t Line;
        size_t Column;
        std::vector<std::string> Context;

        Error(size_t Line, size_t Column, std::vector<std::string>&& Context)
            : Line(Line), Column(Column), Context(std::move(Context)) {}
        virtual ~Error() = default;

        [[nodiscard]] virtual std::string ToString() const = 0;
    };
}

#endif //CVOLT_ERRORS_H