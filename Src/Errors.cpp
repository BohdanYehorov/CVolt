//
// Created by bohdan on 17.12.25.
//

#include "Errors.h"

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

    std::string ParseError::ToString() const
    {
        using enum ParseErrorType;
        switch (Type)
        {
            case ExpectedToken:
                return std::format("Expected '{}', but got '{}'.", Context.at(0), Context.at(1));
            case UnexpectedToken:
                return std::format("Unexpected '{}'.", Context.at(0));
            case UnexpectedEOF:
                return "Unexpected end of file.";
            case ExpectedExpression:
                return "Expected expression.";
            case ExpectedInitializerExpression:
                return "Expected initializer expression after '='.";
            case ExpectedDeclaratorName:
                return "Expected declarator name.";
            case ExpectedFunctionBody:
                return "Expected function body.";
            case ExpectedStatement:
                return "Expected statement.";
            case ExpectedDataType:
                return "Expected data type.";
            case ReturnOutsideFunction:
                return "'return' cannot be used outside of a function.";
            case BreakOutsideLoop:
                return "'break' cannot be used outside of a loop.";
            case ContinueOutsideLoop:
                return "'continue' cannot be used outside of a loop.";
            case FunctionDefinitionNotAllowed:
                return "Function definition is not allowed here.";
            case ExpectedDeclaration:
                return "Expected a declaration.";
            default:
                break;
        }
        return "";
    }
}