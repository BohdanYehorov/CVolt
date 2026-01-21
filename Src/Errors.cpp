//
// Created by bohdan on 17.12.25.
//

#include "Volt/Core/Errors/Errors.h"

namespace Volt
{
    std::string LexError::ToString() const
    {
        using enum LexErrorType;

        switch (Type)
        {
            case InvalidCharacter:
                return std::format("Invalid character '{}'.", Context.at(0));

            case InvalidNumber:
                return std::format("Invalid numeric literal: '{}'.", Context.at(0));

            case UnterminatedNumber:
                return "Unterminated numeric literal.";

            case UnterminatedString:
                return "Unterminated string literal.";

            case InvalidEscape:
                return std::format("Invalid escape sequence '\\{}'.", Context.at(0));

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
                return std::format("Keyword '{}' cannot be used as an identifier.", Context.at(0));

            case UnknownOperator:
                return std::format("Unknown operator '{}'.", Context.at(0));

            case IncompleteOperator:
                return std::format("Incomplete operator '{}'.", Context.at(0));

            case InvalidDelimiter:
                return std::format("Invalid delimiter '{}'.", Context.at(0));

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

    std::string TypeError::ToString() const
    {
        using enum TypeErrorKind;
        switch (Kind)
        {
            case UnknownType:
                return std::format("Type '{}' is not defined.", Context.at(0));
            case InvalidType:
                return std::format("Type '{}' is invalid.", Context.at(0));
            case TypeMissmatch:
                return std::format("Expected '{}', got '{}'.", Context.at(0), Context.at(1));
            case IncompatibleTypes:
                return std::format("Cannot convert '{}' to '{}'.", Context.at(0), Context.at(1));
            case UndefinedVariable:
                return std::format("Variable '{}' is undefined.", Context.at(0));
            case UninitializedVariable:
                return std::format("Variable '{}' is uninitialized.", Context.at(0));
            case Redeclaration:
                return std::format("Redeclaration '{}.'", Context.at(0));
            case ImmutableAssignment:
                return std::format("Cannot assign '{}.'", Context.at(0));
            case InvalidAssignment:
                return std::format("Cannot assign values of this type: '{}'.", Context.at(0));
            case AssignmentTypeMismatch:
                return std::format("Cannot initialize local variable '{}' of type {} with {}.",
                    Context.at(0), Context.at(1), Context.at(2));
            case InvalidBinaryOperator:
                return std::format("Operator '{}' not defined for {}.", Context.at(0), Context.at(1));
            case UndefinedFunction:
                return std::format("Function '{}' is undefined.", Context.at(0));
            default:
                return "";
        }

    }
}
