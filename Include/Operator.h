//
// Created by bohdan on 15.12.25.
//

#ifndef CVOLT_OPERATOR_H
#define CVOLT_OPERATOR_H

#include "Lexer.h"
#include "CompilerTypes.h"


class Operator
{
public:
    enum Type
    {
        UNKNOWN,
        ADD,
        SUB,
        UN_PLS,
        UN_MNS,
        MUL,
        DIV,
        MOD,
        INC,
        DEC,
        ASSIGN,
        ADD_ASSIGN,
        SUB_ASSIGN,
        MUL_ASSIGN,
        DIV_ASSIGN,
        MOD_ASSIGN,
        AND_ASSIGN,
        OR_ASSIGN,
        XOR_ASSIGN,
        LSHIFT_ASSIGN,
        RSHIFT_ASSIGN,
        EQ,
        NEQ,
        GT,
        GTE,
        LT,
        LTE,
        LOGICAL_AND,
        LOGICAL_OR,
        LOGICAL_NOT,
        BIT_AND,
        BIT_OR,
        BIT_XOR,
        BIT_NOT,
        LSHIFT,
        RSHIFT
    };
    using OpMap = std::unordered_map<Token::TokenType, Type>;

    static std::unordered_map<Type, std::string> OperatorStrings;

private:
    static OpMap Assignment;
    static OpMap Logical;
    static OpMap Bitwise;
    static OpMap Equality;
    static OpMap Relational;
    static OpMap Shift;
    static OpMap Additive;
    static OpMap Multiplicative;
    static OpMap Unary;
    static OpMap Postfix;

public:
    static Type GetAssignmentOp(Token::TokenType Op);
    static Type GetLogicalOp(Token::TokenType Op);
    static Type GetBitwiseOp(Token::TokenType Op);
    static Type GetEqualityOp(Token::TokenType Op);
    static Type GetRelationalOp(Token::TokenType Op);
    static Type GetShiftOp(Token::TokenType Op);
    static Type GetAdditiveOp(Token::TokenType Op);
    static Type GetMultiplicativeOp(Token::TokenType Op);
    static Type GetUnaryOp(Token::TokenType Op);
    static Type GetPostfix(Token::TokenType Op);
};


#endif //CVOLT_OPERATOR_H