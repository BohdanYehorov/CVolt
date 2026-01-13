//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_TYPEDVALUE_H
#define CVOLT_TYPEDVALUE_H
#include <llvm/IR/Value.h>
#include "DataType.h"

namespace Volt
{
    class TypedValue : public Object
    {
        GENERATED_BODY(TypedValue, Object)
    private:
        llvm::Value* Value = nullptr;
        DataType* Type = nullptr;
        bool IsLValue = false;

    public:
        TypedValue() = default;
        TypedValue(llvm::Value* Value, DataType* Type, bool IsLValue = false)
            : Value(Value), Type(Type), IsLValue(IsLValue) {}

        [[nodiscard]] llvm::Value* GetValue() const { return Value; }
        [[nodiscard]] DataType* GetDataType() const { return Type; }
    };

    class TypedFunction : public Object
    {
        GENERATED_BODY(TypedValue, Object)
    private:
        llvm::Function* Function = nullptr;
        DataType* ReturnType = nullptr;

    public:
        TypedFunction() = default;
        TypedFunction(llvm::Function* Function, DataType* ReturnType)
            : Function(Function), ReturnType(ReturnType) {}

        [[nodiscard]] llvm::Function* GetFunction() const { return Function; }
        [[nodiscard]] DataType* GetReturnType() const { return ReturnType; }
    };
}


#endif //CVOLT_TYPEDVALUE_H