//
// Created by bohdan on 15.12.25.
//

#ifndef CVOLT_OPERATOR_H
#define CVOLT_OPERATOR_H

#include "Volt/Core/Lexer/Token.h"

namespace Volt
{
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

        static std::string ToString(Type Op);
    };
}

#endif //CVOLT_OPERATOR_H