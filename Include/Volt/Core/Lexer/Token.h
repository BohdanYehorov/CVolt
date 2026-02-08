//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_TOKEN_H
#define CVOLT_TOKEN_H

#include "Volt/Core/Memory/BufferView.h"
#include "Volt/Core/Enums/TokenType.h"

namespace Volt
{
    class CompilationContext;

	struct Token
    {
        TokenType Type = TokenType::UNKNOWN;
        StringRef Lexeme;
        size_t Pos = 0, Line = 1, Column = 1;

        Token() = default;
        Token(TokenType Type, StringRef Lexeme, size_t Pos, size_t Line, size_t Column)
            : Type(Type), Lexeme(Lexeme), Pos(Pos), Line(Line), Column(Column) {}

        [[nodiscard]] std::string ToString(const CompilationContext& Context) const;
    };
}

#endif //CVOLT_TOKEN_H