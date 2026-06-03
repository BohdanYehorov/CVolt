//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_IRValue_H
#define CVOLT_IRValue_H

#include "Volt/Core/Types/DataType.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include "Volt/Core/Enums/OperatorType.h"

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

        [[nodiscard]] IRValue* CastTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] IRValue* CastOrBind(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] llvm::Value* CastLLVM(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);

        IRValue* GetRValue(llvm::IRBuilder<>& Builder, CompilationContext& CContext);

        IRValue* CreateNeg(llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateNot(llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateLogicalNot(llvm::IRBuilder<>& Builder, CompilationContext& CContext);

        IRValue* CreateCmp(IRValue* Right, OperatorType Op, llvm::IRBuilder<>& Builder, CompilationContext& CContext);

        IRValue* CreateAdd(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateSub(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateMul(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateDiv(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateMod(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateBitAnd(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateBitOr(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateBitXor(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateRShift(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        IRValue* CreateLShift(IRValue* Right, llvm::IRBuilder<>& Builder, CompilationContext& CContext);

        IRValue* CreateAssignment(IRValue* Right, OperatorType Op,
            llvm::IRBuilder<>& Builder, CompilationContext& CContext);

    private:
        [[nodiscard]] llvm::Value* CastBooleanTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] llvm::Value* CastCharTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] llvm::Value* CastIntegerTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] llvm::Value* CastFloatTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] llvm::Value* CastPointerTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
        [[nodiscard]] llvm::Value* CastReferenceTo(DataType* To, llvm::IRBuilder<>& Builder, CompilationContext& CContext);
    };
}


#endif //CVOLT_IRValue_H