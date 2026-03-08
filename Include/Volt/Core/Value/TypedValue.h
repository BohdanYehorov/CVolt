//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_TYPEDVALUE_H
#define CVOLT_TYPEDVALUE_H

#include "Volt/Core/Types/DataType.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

namespace Volt
{
    class CompilationContext;

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

        [[nodiscard]] bool CastTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);

    private:
        [[nodiscard]] bool CastBooleanTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastCharTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastIntegerTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastFloatTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastPointerTo(DataType* To, llvm::IRBuilder<>& Builder);
    };
}


#endif //CVOLT_TYPEDVALUE_H