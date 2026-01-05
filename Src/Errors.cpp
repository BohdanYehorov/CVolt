//
// Created by bohdan on 17.12.25.
//

#include "../Include/Errors.h"


std::string LexError::ToString() const
{
    using enum LexErrorType;

    switch (Type)
    {
        case InvalidCharacter:
            return std::format("Invalid character '{}'.", Context[0]);

        case InvalidNumber:
            return "Invalid numeric literal.";

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

std::string ParseError::ToString() const
{
    switch (Code)
    {
        case ErrorCode::ExpectedToken:
            return "Expected '" + Context.at(0) + "'";
        case ErrorCode::UnexpectedToken:
            return "Unexpected '" + Context.at(0) + "'";
        default:
            break;
    }
    return "";
}
