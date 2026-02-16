//
// Created by bohdan on 15.12.25.
//

#ifndef CVOLT_OPERATOR_H
#define CVOLT_OPERATOR_H

#include "Volt/Core/Enums/TokenType.h"
#include "Volt/Core/Enums/OperatorType.h"
#include <string>

namespace Volt
{
    class Operator
    {
    public:
        static OperatorType GetAssignmentOp(TokenType Op);
        static OperatorType GetLogicalOp(TokenType Op);
        static OperatorType GetBitwiseOp(TokenType Op);
        static OperatorType GetEqualityOp(TokenType Op);
        static OperatorType GetRelationalOp(TokenType Op);
        static OperatorType GetShiftOp(TokenType Op);
        static OperatorType GetAdditiveOp(TokenType Op);
        static OperatorType GetMultiplicativeOp(TokenType Op);
        static OperatorType GetUnaryOp(TokenType Op);
        static OperatorType GetPostfix(TokenType Op);

        static std::string ToString(OperatorType Op);
    };
}

#endif //CVOLT_OPERATOR_H