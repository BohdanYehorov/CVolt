//
// Created by bohdan on 8/5/26.
//

#ifndef CVOLT_IRBUILDER_H
#define CVOLT_IRBUILDER_H

#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Core/Types/DataType.h"
#include "Value/IRValue.h"
#include "Volt/Core/Enums/OperatorType.h"
#include <llvm/IR/IRBuilder.h>

namespace Volt
{
    class IRBuilder
    {
    private:
        llvm::IRBuilder<> Builder;
        CompilationContext& CContext;

    public:
        IRBuilder(CompilationContext& CContext)
            : Builder(CContext.Context), CContext(CContext) {}

        IRBuilder(llvm::BasicBlock* InsertBlock, llvm::BasicBlock::iterator InsertBlockIter,
            CompilationContext& CContext) : Builder(InsertBlock, InsertBlockIter), CContext(CContext) {}

        llvm::IRBuilder<>& Get() { return Builder; }

        llvm::AllocaInst* CreateAlloca(DataType* Type);

        IRValue* CreateLoad(IRValue* Value) { return CreateLoad(Value->GetDataType(), Value->GetValue()); }
        IRValue* CreateLoad(DataType* Type, llvm::Value* Value);
        IRValue* CreateLoadIfLValue(IRValue* Value);
        llvm::StoreInst* CreateStore(IRValue* Value, llvm::Value* Ptr);
        llvm::StoreInst* CreateStore(IRValue* Value, IRValue* Ptr);

        IRValue* CreateNeg(IRValue* Value);
        IRValue* CreateNot(IRValue* Value);
        IRValue* CreateLogicalNot(IRValue* Value);

        IRValue* CreateCmp(IRValue* Left, IRValue* Right, OperatorType Op);

        IRValue* CreateAdd(IRValue* Left, IRValue* Right);
        IRValue* CreateSub(IRValue* Left, IRValue* Right);
        IRValue* CreateMul(IRValue* Left, IRValue* Right);
        IRValue* CreateDiv(IRValue* Left, IRValue* Right);
        IRValue* CreateMod(IRValue* Left, IRValue* Right);
        IRValue* CreateAnd(IRValue* Left, IRValue* Right);
        IRValue* CreateOr(IRValue* Left, IRValue* Right);
        IRValue* CreateXor(IRValue* Left, IRValue* Right);
        IRValue* CreateRShift(IRValue* Left, IRValue* Right);
        IRValue* CreateLShift(IRValue* Left, IRValue* Right);
        IRValue* CreateAssignment(IRValue* Left, IRValue* Right, OperatorType Op);

        llvm::ReturnInst* CreateRet(llvm::Value* Value) { return Builder.CreateRet(Value); }
        llvm::ReturnInst* CreateRet(IRValue* Value) { return Builder.CreateRet(Value->GetValue()); }
        llvm::ReturnInst* CreateRetVoid() { return Builder.CreateRetVoid(); }

        llvm::BasicBlock* GetInsertBlock() const { return Builder.GetInsertBlock(); }

        llvm::GlobalVariable* CreateGlobalString(llvm::StringRef Str) { return Builder.CreateGlobalString(Str); }
        IRValue* CreateString(llvm::StringRef Str)
        {
            return CContext.MainArena.Create<IRValue>(CreateGlobalString(Str),
                CContext.GetPointerType(QualType(CContext.GetCharType(), QualType::CONST)));
        }

        llvm::Value* GetInt8(UInt8 Value)  { return Builder.getInt8(Value); }
        llvm::Value* GetInt16(UInt16 Value) { return Builder.getInt16(Value); }
        llvm::Value* GetInt32(UInt32 Value) { return Builder.getInt32(Value); }
        llvm::Value* GetInt64(UInt64 Value) { return Builder.getInt64(Value); }

        IRValue* CreateGEP(IRValue* Value, llvm::Value* Index);
        llvm::Value* CreateGEP(llvm::Type* Ty,
            llvm::Value* Ptr, llvm::ArrayRef<llvm::Value*> IdxList)
        {
            return Builder.CreateGEP(Ty, Ptr, IdxList);
        }

        llvm::BranchInst* CreateCondBr(llvm::Value* Cond,
            llvm::BasicBlock* True, llvm::BasicBlock* False)
        {
            return Builder.CreateCondBr(Cond, True, False);
        }

        llvm::BranchInst* CreateCondBr(IRValue* Cond,
            llvm::BasicBlock* True, llvm::BasicBlock* False)
        {
            return Builder.CreateCondBr(Cond->GetValue(), True, False);
        }

        llvm::BranchInst* CreateBr(llvm::BasicBlock* Block) { return Builder.CreateBr(Block); }

        void SetInsertPoint(llvm::BasicBlock* InsertPoint) { Builder.SetInsertPoint(InsertPoint); }

        llvm::CallInst* CreateCall(llvm::FunctionType* FTy,
            llvm::Value* Callee, llvm::ArrayRef<llvm::Value*> Args)
        {
            return Builder.CreateCall(FTy, Callee, Args);
        }

        llvm::CallInst* CreateCall(llvm::FunctionCallee Callee,
            llvm::ArrayRef<llvm::Value*> Args)
        {
            return Builder.CreateCall(Callee, Args);
        }

        llvm::CallInst* CreateCall(CalleeBase* Callee, llvm::ArrayRef<llvm::Value*> Args,
            std::unique_ptr<llvm::Module>& Module);

        llvm::Value* CreateStructGEP(llvm::Type* Ty,
            llvm::Value* Ptr, unsigned Idx)
        {
            return Builder.CreateStructGEP(Ty, Ptr, Idx);
        }

        llvm::Value* CreateBitCast(
            llvm::Value* V, llvm::Type* DestTy)
        {
            return Builder.CreateBitCast(V, DestTy);
        }

        IRValue* CreateCast(IRValue* V, DataType* DestTy)
        {
            return V->CastTo(DestTy, Builder, CContext);
        }

        IRValue* CreateCastOrBind(IRValue* V, DataType* DestTy);

        llvm::CallInst* CreateMemCpy(IRValue* Dst, IRValue* Src);
        llvm::CallInst* CreateMemCpy(llvm::Value* Dst, llvm::MaybeAlign DstAlign,
            llvm::Value* Src, llvm::MaybeAlign SrcAlign, size_t Size)
        {
            return Builder.CreateMemCpy(Dst, DstAlign, Src, SrcAlign, Size);
        }
    };
}
#endif //CVOLT_IRBUILDER_H
