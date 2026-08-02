//
// Created by bohdan on 23.05.26.
//

#include "Volt/Core/TypeChecker/ExprAddress.h"

namespace Volt
{
    ExprResult* ExprAddress::CreateAssignment(ExprResult *Right, OperatorType Op, CompilationContext &CContext)
    {
        using enum OperatorType;

        if (Type.HasQualifier(QualType::CONST))
            VoltUnreachable("Cannot assign to read only value");

        if (!Value || Value->IsEmpty() || Right->IsEmpty())
        {
            Value = ExprResult::CreateEmpty(Value->GetType(), CContext.MainArena);
            return Value;
        }

        switch (Op)
        {
            case Assign:        Value = Right; break;
            case AddAssign:    Value = Value->CreateAdd(Right, CContext); break;
            case SubAssign:    Value = Value->CreateSub(Right, CContext); break;
            case MulAssign:    Value = Value->CreateMul(Right, CContext); break;
            case DivAssign:    Value = Value->CreateDiv(Right, CContext); break;
            case ModAssign:    Value = Value->CreateMod(Right, CContext); break;
            case AndAssign:    Value = Value->CreateBitAnd(Right, CContext); break;
            case OrAssign:     Value = Value->CreateBitOr(Right, CContext); break;
            case XorAssign:    Value = Value->CreateBitXor(Right, CContext); break;
            case RShiftAssign: Value = Value->CreateBitRShift(Right, CContext); break;
            case LShiftAssign: Value = Value->CreateBitLShift(Right, CContext); break;
            default: VoltUnreachable("Unknown assignment operator");
        }

        return Value;
    }
}
