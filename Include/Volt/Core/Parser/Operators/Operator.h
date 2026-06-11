//
// Created by bohdan on 15.12.25.
//

#ifndef CVOLT_OPERATOR_H
#define CVOLT_OPERATOR_H

#include "Volt/Core/Enums/TokenType.h"
#include "Volt/Core/Enums/OperatorType.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Errors/TypeError.h"
#include <string>

namespace Volt
{
    class CompilationContext;

    enum class BinaryOperatorKind
    {
        Arithmetic,
        Comparison,
        Logical,
        Assignment,
        Unknown
    };

    enum class OperatorErrorKind
    {
        None,
        IncompatibleTypes,
        InvalidOperandTypes
    };

    class Operator
    {
    public:
        [[nodiscard]] static OperatorType GetAssignmentOp(TokenType Op);
        [[nodiscard]] static OperatorType GetLogicalOp(TokenType Op);
        [[nodiscard]] static OperatorType GetBitwiseOp(TokenType Op);
        [[nodiscard]] static OperatorType GetEqualityOp(TokenType Op);
        [[nodiscard]] static OperatorType GetRelationalOp(TokenType Op);
        [[nodiscard]] static OperatorType GetShiftOp(TokenType Op);
        [[nodiscard]] static OperatorType GetAdditiveOp(TokenType Op);
        [[nodiscard]] static OperatorType GetMultiplicativeOp(TokenType Op);
        [[nodiscard]] static OperatorType GetUnaryOp(TokenType Op);
        [[nodiscard]] static OperatorType GetPostfix(TokenType Op);

        [[nodiscard]] static std::string ToString(OperatorType Op);

        [[nodiscard]] static QualType ResolveArithmetic(QualType& Left, QualType& Right, OperatorType Op,
            TypeError& Err);
        [[nodiscard]] static QualType ResolveComparison(QualType& Left, QualType& Right, OperatorType Op,
            TypeError& Err, CompilationContext& CContext);
        [[nodiscard]] static QualType ResolveLogical(QualType& Left, QualType& Right, OperatorType Op,
            TypeError& Err, CompilationContext& CContext);
        [[nodiscard]] static QualType ResolveAssignment(QualType& Left, QualType& Right, OperatorType Op,
            TypeError& Err);
        [[nodiscard]] static QualType ResolvePointerArithmetic(QualType Left, QualType Right, OperatorType Op,
            TypeError& Err);

        [[nodiscard]] static QualType ResolveBinary(QualType& Left, QualType& Right, OperatorType Op,
            TypeError& Err, CompilationContext& CContext);

        [[nodiscard]] static BinaryOperatorKind GetBinaryOperatorKind(OperatorType Op);

        [[nodiscard]] static QualType ResolveUnary(QualType& Operand, OperatorType Op);

    private:
        static bool CastToJointType(QualType& Left, QualType& Right);
        [[nodiscard]] static QualType GetJointType(QualType Left, QualType Right);
        static QualType Normalize(QualType& Left, QualType& Right, QualType Joint);
        [[nodiscard]] static OperatorType GetSecondOpCompoundAssignment(OperatorType Op);
    };
}

#endif //CVOLT_OPERATOR_H