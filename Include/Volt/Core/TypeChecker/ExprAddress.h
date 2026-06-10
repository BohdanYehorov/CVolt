//
// Created by bohdan on 17.05.26.
//

#ifndef CVOLT_EXPRADDRESS_H
#define CVOLT_EXPRADDRESS_H

#include "ExprResult.h"

namespace Volt
{
    class ExprAddress : public SemaResult
    {
        GENERATED_BODY(EmptyAddress, SemaResult)
    private:
        ExprResult* Value;

    public:
        ExprAddress(QualType InType) : SemaResult(InType), Value(nullptr) {}

        ExprAddress(ExprResult* Value) : SemaResult(Value->GetType()), Value(Value) {}

        [[nodiscard]] bool IsEmpty() const { return Value == nullptr; }

        [[nodiscard]] ExprResult* GetValue() const { return Value; }
        void SetValue(ExprResult* NewValue)
        {
            VoltAssert(NewValue->GetType().GetType() == Value->GetType().GetType());
            Value = NewValue;
        }
        ExprResult* CreateAssignment(ExprResult* Right, OperatorType Op, CompilationContext& CContext);
    };
}

#endif //CVOLT_EXPRADDRESS_H
