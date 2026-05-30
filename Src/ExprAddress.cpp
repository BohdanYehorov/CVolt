//
// Created by bohdan on 23.05.26.
//

#include "Volt/Core/TypeChecker/ExprAddress.h"

namespace Volt
{
    ExprResult* ExprAddress::CreateAssignment(ExprResult *Right, OperatorType Op, CompilationContext &CContext)
    {
        using enum OperatorType;

        if (!Value || Value->IsEmpty() || Right->IsEmpty())
        {
            Value = ExprResult::CreateEmpty(Value->GetType(), CContext.MainArena);
            return Value;
        }

        switch (Op)
        {
            case ASSIGN:        Value = Right; break;
            case ADD_ASSIGN:    Value = Value->CreateAdd(Right, CContext); break;
            case SUB_ASSIGN:    Value = Value->CreateSub(Right, CContext); break;
            case MUL_ASSIGN:    Value = Value->CreateMul(Right, CContext); break;
            case DIV_ASSIGN:    Value = Value->CreateDiv(Right, CContext); break;
            case MOD_ASSIGN:    Value = Value->CreateMod(Right, CContext); break;
            case AND_ASSIGN:    Value = Value->CreateBitAnd(Right, CContext); break;
            case OR_ASSIGN:     Value = Value->CreateBitOr(Right, CContext); break;
            case XOR_ASSIGN:    Value = Value->CreateBitXor(Right, CContext); break;
            case RSHIFT_ASSIGN: Value = Value->CreateBitRShift(Right, CContext); break;
            case LSHIFT_ASSIGN: Value = Value->CreateBitLShift(Right, CContext); break;
            default: llvm_unreachable("Unknown assignment operator");
        }

        return Value;
    }
}
