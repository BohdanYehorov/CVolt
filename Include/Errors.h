//
// Created by bohdan on 17.12.25.
//

#ifndef CVOLT_ERRORS_H
#define CVOLT_ERRORS_H

#include <vector>
#include <string>
#include <format>

namespace Volt
{
    enum class LexErrorType
    {
        InvalidCharacter,
        InvalidNumber,
        UnterminatedNumber,

        UnterminatedString,
        InvalidEscape,
        UnterminatedEscape,
        NewlineInString,

        InvalidCharacterLiteral,
        UnterminatedCharacterLiteral,

        UnterminatedBlockComment,
        NestedBlockComment,

        InvalidIdentifier,
        KeywordAsIdentifier,

        UnknownOperator,
        IncompleteOperator,

        InvalidDelimiter,
        UnexpectedEOF,

        InvalidUnicode,
        UnexpectedBOM,

        InternalError,
        TokenCreationFailed,
        InfiniteLoop
    };

    enum class ParseErrorType
    {
        UnexpectedToken,
        ExpectedToken,
        UnexpectedEOF,
        ExpectedExpression,
        ExpectedInitializerExpression,
        ExpectedDeclaratorName,
        ExpectedFunctionBody,
        ExpectedStatement,
        ExpectedDataType,
        ExpectedDeclaration,
        ReturnOutsideFunction,
        BreakOutsideLoop,
        ContinueOutsideLoop,
        FunctionDefinitionNotAllowed
    };

    struct LexError
    {
        LexErrorType Type;
        size_t Line;
        size_t Column;
        std::vector<std::string> Context;

        LexError(LexErrorType Type, size_t Line, size_t Column, std::vector<std::string>&& Context)
            : Type(Type), Line(Line), Column(Column), Context(std::move(Context)) {}

        [[nodiscard]] std::string ToString() const;
    };

    struct ParseError
    {
        ParseErrorType Type;
        size_t Line;
        size_t Column;
        std::vector<std::string> Context;

        ParseError(ParseErrorType Type, size_t Line, size_t Column, const std::vector<std::string>& Context)
            : Type(Type), Line(Line), Column(Column), Context(Context) {}
        [[nodiscard]] std::string ToString() const;
    };
}

#endif //CVOLT_ERRORS_H