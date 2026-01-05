//
// Created by bohdan on 15.12.25.
//

#include "../Include/Operator.h"

std::unordered_map<Operator::Type, std::string> Operator::OperatorStrings = {
    { UNKNOWN, "UNKNOWN" },
    { ADD, "ADD" },
    { SUB, "SUB" },
    { UN_PLS, "UN_PLS" },
    { UN_MNS, "UN_MNS" },
    { MUL, "MUL" },
    { DIV, "DIV" },
    { MOD, "MOD" },
    { INC, "INC" },
    { DEC, "DEC" },
    { ASSIGN, "ASSIGN" },
    { ADD_ASSIGN, "ADD_ASSIGN" },
    { SUB_ASSIGN, "SUB_ASSIGN" },
    { MUL_ASSIGN, "MUL_ASSIGN" },
    { DIV_ASSIGN, "DIV_ASSIGN" },
    { MOD_ASSIGN, "MOD_ASSIGN" },
    { AND_ASSIGN, "AND_ASSIGN" },
    { OR_ASSIGN, "OR_ASSIGN" },
    { XOR_ASSIGN, "XOR_ASSIGN" },
    { LSHIFT_ASSIGN, "LSHIFT_ASSIGN" },
    { RSHIFT_ASSIGN, "RSHIFT_ASSIGN" },
    { EQ, "EQ" },
    { NEQ, "NEQ" },
    { GT, "GT" },
    { GTE, "GTE" },
    { LT, "LT" },
    { LTE, "LTE" },
    { LOGICAL_AND, "LOGICAL_AND" },
    { LOGICAL_OR, "LOGICAL_OR" },
    { LOGICAL_NOT, "LOGICAL_NOT" },
    { BIT_AND, "BIT_AND" },
    { BIT_OR, "BIT_OR" },
    { BIT_XOR, "BIT_XOR" },
    { BIT_NOT, "BIT_NOT" },
    { LSHIFT, "LSHIFT" },
    { RSHIFT, "RSHIFT" }
};

Operator::OpMap Operator::Assignment = {
    { Token::OP_ASSIGN, ASSIGN },
    { Token::OP_ADD_ASSIGN, ADD_ASSIGN },
    { Token::OP_SUB_ASSIGN, SUB_ASSIGN },
    { Token::OP_MUL_ASSIGN, MUL_ASSIGN },
    { Token::OP_DIV_ASSIGN, DIV_ASSIGN },
    { Token::OP_MOD_ASSIGN, MOD_ASSIGN },
    { Token::OP_AND_ASSIGN, AND_ASSIGN },
    { Token::OP_OR_ASSIGN, OR_ASSIGN },
    { Token::OP_XOR_ASSIGN, XOR_ASSIGN },
    { Token::OP_LSHIFT_ASSIGN, LSHIFT_ASSIGN },
    { Token::OP_RSHIFT_ASSIGN, RSHIFT_ASSIGN },
};

Operator::OpMap Operator::Logical = {
    { Token::OP_LOGICAL_AND, LOGICAL_AND },
    { Token::OP_LOGICAL_OR, LOGICAL_OR }
};

Operator::OpMap Operator::Bitwise = {
    { Token::OP_BIT_OR, BIT_OR },
    { Token::OP_BIT_XOR, BIT_XOR },
    { Token::OP_BIT_AND, BIT_AND }
};

Operator::OpMap Operator::Equality = {
    { Token::OP_EQ, EQ },
    { Token::OP_NEQ, NEQ }
};

Operator::OpMap Operator::Relational = {
    { Token::OP_GT, GT },
    { Token::OP_GTE, GTE },
    { Token::OP_LT, LT },
    { Token::OP_LTE, LTE }
};

Operator::OpMap Operator::Shift = {
    { Token::OP_LSHIFT, LSHIFT },
    { Token::OP_RSHIFT, RSHIFT }
};

Operator::OpMap Operator::Additive = {
    { Token::OP_ADD, ADD },
    { Token::OP_SUB, SUB }
};

Operator::OpMap Operator::Multiplicative = {
    { Token::OP_MUL, MUL },
    { Token::OP_DIV, DIV },
    { Token::OP_MOD, MOD }
};

Operator::OpMap Operator::Unary = {
    { Token::OP_ADD, UN_PLS },
    { Token::OP_SUB, UN_MNS },
    { Token::OP_LOGICAL_NOT, LOGICAL_NOT },
    { Token::OP_BIT_NOT, BIT_NOT },
    { Token::OP_INC, INC },
    { Token::OP_DEC, DEC }
};

Operator::OpMap Operator::Postfix = {
    { Token::OP_INC, INC },
    { Token::OP_DEC, DEC }
};

Operator::Type Operator::GetAssignmentOp(Token::TokenType Op)
{
    if (auto Iter = Assignment.find(Op); Iter != Assignment.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetLogicalOp(Token::TokenType Op)
{
    if (auto Iter = Logical.find(Op); Iter != Logical.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetBitwiseOp(Token::TokenType Op)
{
    if (auto Iter = Bitwise.find(Op); Iter != Bitwise.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetEqualityOp(Token::TokenType Op)
{
    if (auto Iter = Equality.find(Op); Iter != Equality.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetRelationalOp(Token::TokenType Op)
{
    if (auto Iter = Relational.find(Op); Iter != Relational.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetShiftOp(Token::TokenType Op)
{
    if (auto Iter = Shift.find(Op); Iter != Shift.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetAdditiveOp(Token::TokenType Op)
{
    if (auto Iter = Additive.find(Op); Iter != Additive.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetMultiplicativeOp(Token::TokenType Op)
{
    if (auto Iter = Multiplicative.find(Op); Iter != Multiplicative.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetUnaryOp(Token::TokenType Op)
{
    if (auto Iter = Unary.find(Op); Iter != Unary.end())
        return Iter->second;
    return UNKNOWN;
}

Operator::Type Operator::GetPostfix(Token::TokenType Op)
{
    if (auto Iter = Postfix.find(Op); Iter != Postfix.end())
        return Iter->second;
    return UNKNOWN;
}
