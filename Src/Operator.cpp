//
// Created by bohdan on 15.12.25.
//

#include "Volt/Core/Parser/Operators/Operator.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Support/ErrorHandling.h"

#define GEN_CASE(Op) case TokenType::OP_##Op: return OperatorType::Op;
#define GEN_CASE_TO_STRING(Op) case OperatorType::Op: return #Op;

namespace Volt
{
    OperatorType Operator::GetAssignmentOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(ASSIGN)
            GEN_CASE(ADD_ASSIGN)
            GEN_CASE(SUB_ASSIGN)
            GEN_CASE(MUL_ASSIGN)
            GEN_CASE(DIV_ASSIGN)
            GEN_CASE(MOD_ASSIGN)
            GEN_CASE(AND_ASSIGN)
            GEN_CASE(OR_ASSIGN)
            GEN_CASE(XOR_ASSIGN)
            GEN_CASE(LSHIFT_ASSIGN)
            GEN_CASE(RSHIFT_ASSIGN)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetLogicalOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(LOGICAL_AND)
            GEN_CASE(LOGICAL_OR)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetBitwiseOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(BIT_AND)
            GEN_CASE(BIT_OR)
            GEN_CASE(BIT_XOR)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetEqualityOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(EQ)
            GEN_CASE(NEQ)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetRelationalOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(GT)
            GEN_CASE(GTE)
            GEN_CASE(LT)
            GEN_CASE(LTE)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetShiftOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(LSHIFT)
            GEN_CASE(RSHIFT)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetAdditiveOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(ADD)
            GEN_CASE(SUB)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetMultiplicativeOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(MUL)
            GEN_CASE(DIV)
            GEN_CASE(MOD)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetUnaryOp(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(ADD)
            GEN_CASE(SUB)
            GEN_CASE(LOGICAL_NOT)
            GEN_CASE(BIT_NOT)
            GEN_CASE(INC)
            GEN_CASE(DEC)
            GEN_CASE(MUL)
            default: return OperatorType::UNKNOWN;
        }
    }

    OperatorType Operator::GetPostfix(TokenType Op)
    {
        switch (Op)
        {
            GEN_CASE(INC)
            GEN_CASE(DEC)
            default: return OperatorType::UNKNOWN;
        }
    }

    std::string Operator::ToString(OperatorType Op)
    {
        switch (Op)
        {
            GEN_CASE_TO_STRING(UNKNOWN)
            GEN_CASE_TO_STRING(ADD)
            GEN_CASE_TO_STRING(SUB)
            GEN_CASE_TO_STRING(MUL)
            GEN_CASE_TO_STRING(DIV)
            GEN_CASE_TO_STRING(MOD)
            GEN_CASE_TO_STRING(INC)
            GEN_CASE_TO_STRING(DEC)
            GEN_CASE_TO_STRING(ASSIGN)
            GEN_CASE_TO_STRING(ADD_ASSIGN)
            GEN_CASE_TO_STRING(SUB_ASSIGN)
            GEN_CASE_TO_STRING(MUL_ASSIGN)
            GEN_CASE_TO_STRING(DIV_ASSIGN)
            GEN_CASE_TO_STRING(MOD_ASSIGN)
            GEN_CASE_TO_STRING(AND_ASSIGN)
            GEN_CASE_TO_STRING(OR_ASSIGN)
            GEN_CASE_TO_STRING(XOR_ASSIGN)
            GEN_CASE_TO_STRING(LSHIFT_ASSIGN)
            GEN_CASE_TO_STRING(RSHIFT_ASSIGN)
            GEN_CASE_TO_STRING(EQ)
            GEN_CASE_TO_STRING(NEQ)
            GEN_CASE_TO_STRING(GT)
            GEN_CASE_TO_STRING(GTE)
            GEN_CASE_TO_STRING(LT)
            GEN_CASE_TO_STRING(LTE)
            GEN_CASE_TO_STRING(LOGICAL_AND)
            GEN_CASE_TO_STRING(LOGICAL_OR)
            GEN_CASE_TO_STRING(LOGICAL_NOT)
            GEN_CASE_TO_STRING(BIT_AND)
            GEN_CASE_TO_STRING(BIT_OR)
            GEN_CASE_TO_STRING(BIT_XOR)
            GEN_CASE_TO_STRING(BIT_NOT)
            GEN_CASE_TO_STRING(LSHIFT)
            GEN_CASE_TO_STRING(RSHIFT)
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

        TypeCategory Category = JointType->GetCategory();

        switch (Op)
        {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            {
                if (Category == INTEGER || Category == FLOATING_POINT)
                    return Normalize(Left, Right, JointType);

                Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
                Err.Context = { JointType->ToString() };
                return {};
            }

            case MOD:
            case BIT_AND:
            case BIT_OR:
            case BIT_XOR:
            case LSHIFT:
            case RSHIFT:
            {
                if (Category == INTEGER)
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

        TypeCategory Category = JointType->GetCategory();

        switch (Op)
        {
            case EQ:
            case NEQ:
            {
                if (Category == BOOLEAN ||
                    Category == CHAR ||
                    Category == INTEGER ||
                    Category == FLOATING_POINT ||
                    Category == POINTER)
                {
                    Normalize(Left, Right, JointType);
                    return { CContext.GetBoolType(), 0 };
                }

                Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
                Err.Context = { JointType->ToString() };
                return {};
            }

            case GT:
            case GTE:
            case LT:
            case LTE:
            {
                if (Category == CHAR ||
                    Category == INTEGER ||
                    Category == FLOATING_POINT)
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
            case LOGICAL_AND:
            case LOGICAL_OR:
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

        TypeCategory Category = Left->GetCategory();

        switch (Op)
        {
            case ASSIGN:
            {
                if (Category != VOID)
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
                if (SecondOp == UNKNOWN)
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
        TypeError& Err, CompilationContext& CContext)
    {
        using enum TypeCategory;
        using enum OperatorType;

        TypeCategory LeftCategory = Left->GetCategory();
        TypeCategory RightCategory = Right->GetCategory();

        if (LeftCategory == POINTER && RightCategory == INTEGER)
        {
            if (Op == ADD || Op == SUB)
                return Left;

            Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
            Err.Context = { Left->ToString(), Right->ToString() };
            return {};
        }

        if (LeftCategory == INTEGER && RightCategory == POINTER)
        {
            if (Op == ADD)
                return Right;

            Err.Kind = TypeErrorKind::BinaryOperandTypeMismatch;
            Err.Context = { Left->ToString(), Right->ToString() };
            return {};
        }

        // if (LeftCategory == POINTER && RightCategory == POINTER)
        // {
        //     if (Op != SUB)
        //         return {};
        //
        //     if (Cast<PointerType>(Left.GetType())->BaseType != Cast<PointerType>(Right.GetType())->BaseType)
        //         return {};
        //
        //     return { CContext.GetIntegerType(64), 0 };
        // }

        return {};
    }

    QualType Operator::ResolveBinary(QualType &Left, QualType &Right, OperatorType Op,
        TypeError& Err, CompilationContext& CContext)
    {
        switch (GetBinaryOperatorKind(Op))
        {
            case BinaryOperatorKind::Arithmetic:
                if (QualType PtrArithmeticResType = ResolvePointerArithmetic(Left, Right, Op, Err, CContext))
                    return PtrArithmeticResType;
                return ResolveArithmetic(Left, Right, Op, Err);
            case BinaryOperatorKind::Comparison:
                return ResolveComparison(Left, Right, Op, Err, CContext);
            case BinaryOperatorKind::Logical:
                return ResolveLogical(Left, Right, Op, Err, CContext);
            case BinaryOperatorKind::Assignment:
                return ResolveAssignment(Left, Right, Op, Err);
            default:
                return {};
        }
    }

    BinaryOperatorKind Operator::GetBinaryOperatorKind(OperatorType Op)
    {
        using enum OperatorType;

        switch (Op)
        {
            case ADD:
            case SUB:
            case MUL:
            case DIV:
            case MOD:
            case BIT_AND:
            case BIT_OR:
            case BIT_XOR:
            case LSHIFT:
            case RSHIFT:
                return BinaryOperatorKind::Arithmetic;

            case EQ:
            case NEQ:
            case GT:
            case GTE:
            case LT:
            case LTE:
                return BinaryOperatorKind::Comparison;

            case LOGICAL_AND:
            case LOGICAL_OR:
                return BinaryOperatorKind::Logical;

            case ADD_ASSIGN:
            case SUB_ASSIGN:
            case MUL_ASSIGN:
            case DIV_ASSIGN:
            case MOD_ASSIGN:
            case AND_ASSIGN:
            case OR_ASSIGN:
            case XOR_ASSIGN:
            case LSHIFT_ASSIGN:
            case RSHIFT_ASSIGN:
                return BinaryOperatorKind::Assignment;

            default:
                return BinaryOperatorKind::Unknown;
        }
    }

    QualType Operator::ResolveUnary(QualType &Operand, OperatorType Op)
    {
        using enum TypeCategory;
        using enum OperatorType;

        TypeCategory Category = Operand->GetCategory();

        switch (Op)
        {
            case ADD:
            case SUB:
            case INC:
            case DEC:
            {
                if (Category == CHAR || Category == INTEGER || Category == FLOATING_POINT)
                    return Operand;

                return {};
            }

            case BIT_NOT:
            {
                if (Category == CHAR || Category == INTEGER)
                    return Operand;

                return {};
            }

            case LOGICAL_NOT:
            {
                if (Category == BOOLEAN)
                    return Operand;

                return {};
            }

            default:
                return {};
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
        // Src = Src->ImplicitCast(Dst);
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
            case ADD_ASSIGN: return ADD;
            case SUB_ASSIGN: return SUB;
            case MUL_ASSIGN: return MUL;
            case DIV_ASSIGN: return DIV;
            case MOD_ASSIGN: return MOD;
            case AND_ASSIGN: return BIT_AND;
            case OR_ASSIGN: return BIT_OR;
            case XOR_ASSIGN: return BIT_XOR;
            case LSHIFT_ASSIGN: return LSHIFT;
            case RSHIFT_ASSIGN: return RSHIFT;
            default: return UNKNOWN;
        }
    }
}
#undef GEN_CASE
#undef GEN_CASE_TO_STRING
