//
// Created by bohdan on 15.12.25.
//

#include "Volt/Core/Parser/Operators/Operator.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Support/ErrorHandling.h"

#define GEN_CASE(Tok, Op) case TokenType::Tok: return OperatorType::Op;
#define GEN_CASE_TO_STRING(Op) case OperatorType::Op: return #Op;

namespace Volt
{
    OperatorType Operator::GetAssignmentOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(Equal, Assign)
            GEN_CASE(PlusEqual, AddAssign)
            GEN_CASE(MinusEqual, SubAssign)
            GEN_CASE(StarEqual, MulAssign)
            GEN_CASE(SlashEqual, DivAssign)
            GEN_CASE(PercentEqual, ModAssign)
            GEN_CASE(AmpEqual, AndAssign)
            GEN_CASE(PipeEqual, OrAssign)
            GEN_CASE(CaretEqual, XorAssign)
            GEN_CASE(LessLessEqual, LShiftAssign)
            GEN_CASE(GraterGraterEqual, RShiftAssign)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetLogicalOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(AmpAmp, LogicalAnd)
            GEN_CASE(PipePipe, LogicalOr)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetBitwiseOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(Amp, BitAnd)
            GEN_CASE(Pipe, BitOr)
            GEN_CASE(Caret, BitXor)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetEqualityOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(EqualEqual, Equal)
            GEN_CASE(ExclaimEqual, NotEqual)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetRelationalOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(Grater, Grater)
            GEN_CASE(GraterEqual, GraterEqual)
            GEN_CASE(Less, Less)
            GEN_CASE(LessEqual, LessEqual)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetShiftOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(LessLess, LShift)
            GEN_CASE(GraterGrater, RShift)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetAdditiveOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(Plus, Add)
            GEN_CASE(Minus, Sub)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetMultiplicativeOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(Star, Mul)
            GEN_CASE(Slash, Div)
            GEN_CASE(Percent, Mod)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetUnaryOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(Plus, UnPlus)
            GEN_CASE(Minus, UnMinus)
            GEN_CASE(Exclaim, LogicalNot)
            GEN_CASE(Tilde, BitNot)
            GEN_CASE(PlusPlus, Inc)
            GEN_CASE(MinusMinus, Dec)
            GEN_CASE(Star, Unref)
            default: return OperatorType::Unknown;
        }
    }

    OperatorType Operator::GetPostfix(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(PlusPlus, Inc)
            GEN_CASE(MinusMinus, Dec)
            default: return OperatorType::Unknown;
        }
    }

    std::string Operator::ToString(OperatorType Op)
    {
        switch (Op)
        {
            GEN_CASE_TO_STRING(Add)
            GEN_CASE_TO_STRING(Sub)
            GEN_CASE_TO_STRING(Mul)
            GEN_CASE_TO_STRING(Div)
            GEN_CASE_TO_STRING(Mod)
            GEN_CASE_TO_STRING(Inc)
            GEN_CASE_TO_STRING(Dec)
            GEN_CASE_TO_STRING(Assign)
            GEN_CASE_TO_STRING(AddAssign)
            GEN_CASE_TO_STRING(SubAssign)
            GEN_CASE_TO_STRING(MulAssign)
            GEN_CASE_TO_STRING(DivAssign)
            GEN_CASE_TO_STRING(ModAssign)
            GEN_CASE_TO_STRING(AndAssign)
            GEN_CASE_TO_STRING(OrAssign)
            GEN_CASE_TO_STRING(XorAssign)
            GEN_CASE_TO_STRING(LShiftAssign)
            GEN_CASE_TO_STRING(RShiftAssign)
            GEN_CASE_TO_STRING(Equal)
            GEN_CASE_TO_STRING(NotEqual)
            GEN_CASE_TO_STRING(Grater)
            GEN_CASE_TO_STRING(GraterEqual)
            GEN_CASE_TO_STRING(Less)
            GEN_CASE_TO_STRING(LessEqual)
            GEN_CASE_TO_STRING(LogicalAnd)
            GEN_CASE_TO_STRING(LogicalOr)
            GEN_CASE_TO_STRING(LogicalNot)
            GEN_CASE_TO_STRING(BitAnd)
            GEN_CASE_TO_STRING(BitOr)
            GEN_CASE_TO_STRING(BitXor)
            GEN_CASE_TO_STRING(BitNot)
            GEN_CASE_TO_STRING(LShift)
            GEN_CASE_TO_STRING(RShift)
            GEN_CASE_TO_STRING(UnPlus)
            GEN_CASE_TO_STRING(UnMinus)
            GEN_CASE_TO_STRING(Unref)
            GEN_CASE_TO_STRING(Unknown)
            default: return "?";
        }
    }

    QualType Operator::ResolveArithmetic(QualType &Left, QualType &Right,
        OperatorType Op, TypeError& Err)
    {
        using enum TypeCategory;
        using enum OperatorType;

        QualType JointType = GetJointType(Left, Right);
        if (!JointType)
        {
            Err.Kind = TypeErrorKind::IncompatibleTypes;
            Err.Context = { Left->ToString(), Right->ToString() };
            return {};
        }

        switch (Op)
        {
            case Add: case Sub:
            case Mul: case Div:
            {
                if (JointType->IsOneOf(INTEGER, FLOATING_POINT))
                    return Normalize(Left, Right, JointType);

                Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
                Err.Context = { JointType->ToString() };
                return {};
            }

            case Mod:    case BitAnd:
            case BitOr: case BitXor:
            case LShift: case RShift:
            {
                if (JointType->IsIntegerType())
                    return Normalize(Left, Right, JointType);

                Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
                Err.Context = { JointType->ToString() };
                return {};
            }
            default:
                VoltUnreachable("Invalid arithmetic operator");
        }
    }

    QualType Operator::ResolveComparison(QualType &Left, QualType &Right, OperatorType Op,
        TypeError& Err, CompilationContext& CContext)
    {
        using enum TypeCategory;
        using enum OperatorType;

        QualType JointType = GetJointType(Left, Right);
        if (!JointType)
        {
            Err.Kind = TypeErrorKind::IncompatibleTypes;
            Err.Context = { Left->ToString(), Right->ToString() };
            return {};
        }

        switch (Op)
        {
            case Equal: case NotEqual:
            {
                if (JointType->IsOneOf(BOOLEAN, CHAR, INTEGER, FLOATING_POINT, POINTER))
                {
                    Normalize(Left, Right, JointType);
                    return { CContext.GetBoolType(), 0 };
                }

                Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
                Err.Context = { JointType->ToString() };
                return {};
            }

            case Grater: case GraterEqual:
            case Less:   case LessEqual:
            {
                if (JointType->IsOneOf(INTEGER, FLOATING_POINT))
                {
                    Normalize(Left, Right, JointType);
                    return { CContext.GetBoolType(), 0 };
                }

                Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
                Err.Context = { JointType->ToString() };
                return {};
            }

            default:
                VoltUnreachable("Invalid comparison operator");
        }
    }

    QualType Operator::ResolveLogical(QualType &Left, QualType &Right, OperatorType Op,
        TypeError& Err, CompilationContext& CContext)
    {
        using enum TypeCategory;
        using enum OperatorType;

        DataType* BoolTy = CContext.GetBoolType();
        if (!Left->ImplicitCast(BoolTy))
        {
            Err.Kind = TypeErrorKind::IncompatibleTypes;
            Err.Context = { Left->ToString(), BoolTy->ToString() };
            return {};
        }

        if (!Right->ImplicitCast(BoolTy))
        {
            Err.Kind = TypeErrorKind::IncompatibleTypes;
            Err.Context = { Right->ToString(), BoolTy->ToString() };
            return {};
        }

        switch (Op)
        {
            case LogicalAnd:
            case LogicalOr:
                return Normalize(Left, Right, QualType(BoolTy, 0));

            default:
                VoltUnreachable("Invalid logical operator");
        }
    }

    QualType Operator::ResolveAssignment(QualType &Left, QualType &Right,
                                         OperatorType Op, TypeError& Err)
    {
        using enum TypeCategory;
        using enum OperatorType;

        if (!Right.ImplicitCast(Left))
        {
            Err.Kind = TypeErrorKind::IncompatibleTypes;
            Err.Context = { Left->ToString(), Right->ToString() };
            return {};
        }

        switch (Op)
        {
            case Assign:
            {
                if (!Left->IsVoidType())
                {
                    Right = Left;
                    return Left;
                }

                Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
                Err.Context = { Left->ToString() };
            }

            default:
            {
                OperatorType SecondOp = GetSecondOpCompoundAssignment(Op);
                if (SecondOp == Unknown)
                    VoltUnreachable("Unknown assignment operator");

                QualType TmpLeft = Left, TmpRight = Right;
                QualType Result = ResolveArithmetic(TmpLeft, TmpRight, SecondOp, Err);
                if (!Result) return {};

                if (!Result.ImplicitCast(Left))
                {
                    Err.Kind = TypeErrorKind::IncompatibleTypes;
                    Err.Context = { Left->ToString(), Right->ToString() };
                }

                Right = Left;
                return Left;
            }
        }
    }

    QualType Operator::ResolvePointerArithmetic(QualType Left, QualType Right, OperatorType Op,
        TypeError& Err)
    {
        using enum TypeCategory;
        using enum OperatorType;

        TypeCategory LeftCategory = Left->GetCategory();
        TypeCategory RightCategory = Right->GetCategory();

        if (LeftCategory == POINTER && RightCategory == INTEGER)
        {
            if (Op == Add || Op == Sub)
                return Left;

            Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
            Err.Context = { Left->ToString(), Right->ToString() };
            return {};
        }

        if (LeftCategory == INTEGER && RightCategory == POINTER)
        {
            if (Op == Add)
                return Right;

            Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
            Err.Context = { Left->ToString(), Right->ToString() };
            return {};
        }

        return {};
    }

    QualType Operator::ResolveBinary(QualType &Left, QualType &Right, OperatorType Op,
        TypeError& Err, CompilationContext& CContext)
    {
        switch (GetBinaryOperatorKind(Op))
        {
            case BinaryOperatorKind::Arithmetic:
                if (QualType PtrArithmeticResType = ResolvePointerArithmetic(Left, Right, Op, Err))
                    return PtrArithmeticResType;
                return ResolveArithmetic(Left, Right, Op, Err);
            case BinaryOperatorKind::Comparison:
                return ResolveComparison(Left, Right, Op, Err, CContext);
            case BinaryOperatorKind::Logical:
                return ResolveLogical(Left, Right, Op, Err, CContext);
            case BinaryOperatorKind::Assignment:
                return ResolveAssignment(Left, Right, Op, Err);
            default:
                VoltUnreachable("Invalid binary operator kind");
        }
    }

    BinaryOperatorKind Operator::GetBinaryOperatorKind(OperatorType Op)
    {
        using enum OperatorType;

        switch (Op)
        {
            case Add:     case Sub:    case Mul:
            case Div:     case Mod:
            case BitAnd:  case BitOr:  case BitXor:
            case LShift:  case RShift:
                return BinaryOperatorKind::Arithmetic;

            case Equal:  case NotEqual:
            case Grater: case GraterEqual:
            case Less:   case LessEqual:
                return BinaryOperatorKind::Comparison;

            case LogicalAnd: case LogicalOr:
                return BinaryOperatorKind::Logical;

            case Assign:
            case AddAssign:    case SubAssign:
            case MulAssign:    case DivAssign:
            case ModAssign:    case AndAssign:
            case OrAssign:     case XorAssign:
            case LShiftAssign: case RShiftAssign:
                return BinaryOperatorKind::Assignment;

            default:
                return BinaryOperatorKind::Unknown;
        }
    }

    QualType Operator::ResolveUnary(QualType &Operand, OperatorType Op, TypeError& Err)
    {
        using enum TypeCategory;
        using enum OperatorType;

        switch (Op)
        {
            case UnPlus:
            case Inc: case Dec:
            {
                if (Operand->IsOneOf(INTEGER, FLOATING_POINT))
                    return Operand;

                Err.Kind = TypeErrorKind::UnaryOperandTypeMismatch;
                Err.Context = { Operand.ToString() };
                return {};
            }

            case UnMinus:
            {
                if (Operand->IsSignedIntegerType() || Operand->IsFloatingPointType())
                    return Operand;

                Err.Kind = TypeErrorKind::UnaryOperandTypeMismatch;
                Err.Context = { Operand.ToString() };
                return {};
            }

            case BitNot:
            {
                if (Operand->IsIntegerType())
                    return Operand;

                Err.Kind = TypeErrorKind::UnaryOperandTypeMismatch;
                Err.Context = { Operand.ToString() };
                return {};
            }

            case LogicalNot:
            {
                if (Operand->IsBoolType())
                    return Operand;

                Err.Kind = TypeErrorKind::UnaryOperandTypeMismatch;
                Err.Context = { Operand.ToString() };
                return {};
            }

            default:
                VoltUnreachable("Invalid unary operator");
        }
    }

    bool Operator::CastToJointType(QualType &Left, QualType &Right)
    {
        if (Left == Right) return true;

        int LeftRank = Left->GetRank();
        int RightRank = Right->GetRank();

        if (LeftRank == -1 || RightRank == -1)
            return false;

        QualType& Dst = LeftRank > RightRank ? Left : Right;
        QualType& Src = Dst == Left ? Right : Left;
        if (!Src.ImplicitCast(Dst)) return false;
        Src = Dst;

        return true;
    }

    QualType Operator::GetJointType(QualType Left, QualType Right)
    {
        if (CastToJointType(Left, Right))
            return Left;
        return {};
    }

    QualType Operator::Normalize(QualType &Left, QualType &Right, QualType Joint)
    {
        Left = Joint;
        Right = Joint;
        return Joint;
    }

    OperatorType Operator::GetSecondOpCompoundAssignment(OperatorType Op)
    {
        using enum OperatorType;

        switch (Op)
        {
            case AddAssign:    return Add;
            case SubAssign:    return Sub;
            case MulAssign:    return Mul;
            case DivAssign:    return Div;
            case ModAssign:    return Mod;
            case AndAssign:    return BitAnd;
            case OrAssign:     return BitOr;
            case XorAssign:    return BitXor;
            case LShiftAssign: return LShift;
            case RShiftAssign: return RShift;
            default:           return Unknown;
        }
    }
}
#undef GEN_CASE
#undef GEN_CASE_TO_STRING
