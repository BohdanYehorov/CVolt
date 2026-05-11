//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_IRValue_H
#define CVOLT_IRValue_H

#include "Volt/Core/Types/DataType.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

namespace Volt
{
    class CompilationContext;

    class IRValue : public Object
    {
        GENERATED_BODY(IRValue, Object)
    private:
        llvm::Value* Value = nullptr;
        DataType* Type = nullptr;
        bool bIsLValue = false;

    public:
        IRValue() = default;
        IRValue(DataType* Type, bool IsLValue = false)
            : Type(Type), bIsLValue(IsLValue) {}
        IRValue(llvm::Value* Value, DataType* Type, bool IsLValue = false)
            : Value(Value), Type(Type), bIsLValue(IsLValue) {}

        IRValue(llvm::Value* Value, DataType* Type, llvm::IRBuilder<>& Builder);

        [[nodiscard]] llvm::Value* GetValue() const { return Value; }
        [[nodiscard]] DataType* GetDataType() const { return Type; }
        [[nodiscard]] bool IsLValue() const { return bIsLValue; }

        [[nodiscard]] bool CastTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);

        bool ToRValue(llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* GetRValue(llvm::IRBuilder<>& Builder, CompilationContext& CContext);

    private:
        [[nodiscard]] bool CastBooleanTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastCharTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastIntegerTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastFloatTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] bool CastPointerTo(DataType* To, llvm::IRBuilder<>& Builder);
        [[nodiscard]] bool CastReferenceTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
    };
}


#endif //CVOLT_IRValue_H