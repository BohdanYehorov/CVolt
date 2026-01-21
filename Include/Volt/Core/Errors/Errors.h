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

    enum class TypeErrorKind
    {
        UnknownType,
        InvalidType,
        TypeMissmatch,
        IncompatibleTypes,
        UndefinedVariable,
        UninitializedVariable,
        Redeclaration,
        ImmutableAssignment,
        InvalidAssignment,
        AssignmentTypeMismatch,
        InvalidBinaryOperator,
        BinaryOperandTypeMismatch,
        LogicalOperatorOnNonBool,
        ComparisonTypeMismatch,
        InvalidUnaryOperator,
        UnaryOperandTypeMismatch,
        ConditionNotBool,
        DuplicateFunction,
        InvalidReturnType,
        UndefinedFunction,
        ArgumentCountMismatch,
        ArgumentTypeMismatch,
        AmbiguousFunctionCall,
        InvalidCalleeType,
        CallingNonCallable,
        ReturnTypeMismatch,
        MissingReturn,
        VoidReturnValue,
        NonVoidMissingReturn,
        IndexingNonArray,
        IndexNotInteger,
        ArrayElementTypeMismatch,
        InvalidArrayLiteral
    };

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

    struct LexError : Error
    {
        LexErrorType Type;
        LexError(LexErrorType Type, size_t Line, size_t Column, std::vector<std::string>&& Context)
            : Error(Line, Column, std::move(Context)), Type(Type) {}

        [[nodiscard]] std::string ToString() const override;
    };

    struct ParseError : Error
    {
        ParseErrorType Type;
        ParseError(ParseErrorType Type, size_t Line, size_t Column, std::vector<std::string>&& Context)
            : Error(Line, Column, std::move(Context)), Type(Type) {}
        [[nodiscard]] std::string ToString() const override;
    };

    struct TypeError : Error
    {
        TypeErrorKind Kind;
        TypeError(TypeErrorKind Kind, size_t Line, size_t Column, std::vector<std::string>&& Context)
            : Error(Line, Column, std::move(Context)), Kind(Kind) {}
        [[nodiscard]] std::string ToString() const override;
    };
}

#endif //CVOLT_ERRORS_H