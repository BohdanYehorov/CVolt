//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_TYPEDVALUE_H
#define CVOLT_TYPEDVALUE_H
#include "Volt/Core/Types/DataType.h"
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
}


#endif //CVOLT_TYPEDVALUE_H