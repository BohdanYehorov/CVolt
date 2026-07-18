//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_TOKEN_H
#define CVOLT_TOKEN_H

#include "Volt/Core/Enums/TokenType.h"

namespace Volt
{
    class CompilationContext;

	struct StringRef
	{
		size_t Index = 0;
		size_t Length = 0;

		StringRef() = default;
		StringRef(size_t Index, size_t Length)
			: Index(Index), Length(Length) {}
	};

	struct Token
    {
        TokenType Type = TokenType::UNKNOWN;
        StringRef Lexeme;
        size_t Pos = 0, Line = 1, Column = 1;

        Token() = default;
        Token(TokenType Type, StringRef Lexeme, size_t Pos, size_t Line, size_t Column)
            : Type(Type), Lexeme(Lexeme), Pos(Pos), Line(Line), Column(Column) {}

        [[nodiscard]] std::string ToString(const CompilationContext& Context) const;

	    template <typename ...Args_>
	    [[nodiscard]] bool IsOneOf(Args_... Types) const
	    {
	    	static_assert((std::same_as<Args_, TokenType> && ...));
	    	return ((Types == Type) || ...);
	    }
    };
}

#endif //CVOLT_TOKEN_H