//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_TYPEDVALUE_H
#define CVOLT_TYPEDVALUE_H
#include "Volt/Core/Functions/Callee.h"
#include "DataTypeUtils.h"
#include <llvm/IR/Value.h>

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
        TypedValue(DataType* Type, bool IsLValue = false)
            : Type(Type), IsLValue(IsLValue) {}
        TypedValue(llvm::Value* Value, DataType* Type, bool IsLValue = false)
            : Value(Value), Type(Type), IsLValue(IsLValue) {}

        [[nodiscard]] llvm::Value* GetValue() const { return Value; }
        [[nodiscard]] DataType* GetDataType() const { return Type; }

        void InitValue(llvm::Value* InValue)
        {
            if (!Value)
                Value = InValue;
            else
                throw std::runtime_error("Value has already initialized.");
        }
    };

    class TypedFunction : public Object
    {
        GENERATED_BODY(TypedValue, Object)
    private:
        llvm::Function* Function = nullptr;
        DataType* ReturnType = nullptr;

    public:
        TypedFunction() = default;
        TypedFunction(DataType* ReturnType)
            : ReturnType(ReturnType) {}
        TypedFunction(llvm::Function* Function, DataType* ReturnType)
            : Function(Function), ReturnType(ReturnType) {}

        [[nodiscard]] llvm::Function* GetFunction() const { return Function; }
        [[nodiscard]] DataType* GetReturnType() const { return ReturnType; }

        void InitFunction(llvm::Function* InFunction)
        {
            if (!Function)
                Function = InFunction;
            else
                throw std::runtime_error("Function has already initialized.");
        }
    };
}


#endif //CVOLT_TYPEDVALUE_H