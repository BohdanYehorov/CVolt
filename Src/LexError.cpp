//
// Created by bohdan on 07.02.26.
//

#include "Volt/Core/Errors/LexError.h"

namespace Volt
{
    std::string LexError::ToString() const
    {
        using enum LexErrorType;

        switch (Type)
        {
            case InvalidCharacter:
                return std::format("Invalid character '{}'.", Context[0]);

            case InvalidNumber:
                return std::format("Invalid numeric literal: '{}'.", Context[0]);

            case UnterminatedNumber:
                return "Unterminated numeric literal.";

            case UnterminatedString:
                return "Unterminated string literal.";

            case InvalidEscape:
                return std::format("Invalid escape sequence '\\{}'.", Context[0]);

            case UnterminatedEscape:
                return "Unterminated escape sequence in string literal.";

            case NewlineInString:
                return "Newline in string literal.";

            case InvalidCharacterLiteral:
                return "Invalid character literal.";

            case UnterminatedCharacterLiteral:
                return "Unterminated character literal.";

            case UnterminatedBlockComment:
                return "Unterminated block comment.";

            case NestedBlockComment:
                return "Nested block comments are not supported.";

            case InvalidIdentifier:
                return "Invalid identifier.";

            case KeywordAsIdentifier:
                return std::format("Keyword '{}' cannot be used as an identifier.", Context[0]);

            case UnknownOperator:
                return std::format("Unknown operator '{}'.", Context[0]);

            case IncompleteOperator:
                return std::format("Incomplete operator '{}'.", Context[0]);

            case InvalidDelimiter:
                return std::format("Invalid delimiter '{}'.", Context[0]);

            case UnexpectedEOF:
                return "Unexpected end of file.";

            case InvalidUnicode:
                return "Invalid Unicode character.";

            case UnexpectedBOM:
                return "Unexpected byte order mark (BOM).";

            case InternalError:
                return "Internal lexer error.";

            case TokenCreationFailed:
                return "Failed to create token.";

            case InfiniteLoop:
                return "Lexer entered an infinite loop.";
        }

        return "Unknown lexer error.";
    }
}