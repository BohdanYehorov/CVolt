//
// Created by bohdan on 17.12.25.
//

#ifndef CVOLT_ERRORS_H
#define CVOLT_ERRORS_H

#include <vector>
#include <string>
#include <format>
#include <unordered_map>

enum class ErrorCode
{
    InvalidCharacter,
    InvalidNumber,
    UnterminatedString,
    InvalidEscape,
    UnterminatedComment,
    InvalidIdentifier,
    UnknownOperator,
    InternalError,

    UnexpectedToken,
    ExpectedToken,
    ExpectedExpression,
    ExpectedIdentifier,
    ExpectedType,
    DuplicateParameter
};

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
    ErrorCode Code;
    size_t Pos;
    std::vector<std::string> Context;

    ParseError(ErrorCode Code, size_t Pos, const std::vector<std::string>& Context)
        : Code(Code), Pos(Pos), Context(Context) {}
    [[nodiscard]] std::string ToString() const;
};

#endif //CVOLT_ERRORS_H