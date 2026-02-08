//
// Created by bohdan on 15.12.25.
//

#include "Volt/Core/Parser/Operators/Operator.h"

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
            GEN_CASE_TO_STRING(UN_PLS)
            GEN_CASE_TO_STRING(UN_MNS)
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
}

#undef GEN_CASE
#undef GEN_CASE_TO_STRING
