//
// Created by bohdan on 8/5/26.
//

#include "Volt/Compiler/IRBuilder.h"

namespace Volt
{
    llvm::AllocaInst *IRBuilder::CreateAlloca(DataType *Type)
    {
        llvm::AllocaInst* Alloca = Builder.CreateAlloca(CContext.GetLLVMType(Type));
        Alloca->setAlignment(llvm::Align(Type->GetAlignment()));
        return Alloca;
    }

    IRValue * IRBuilder::CreateLoad(DataType *Type, llvm::Value *Value)
    {
        llvm::Type* LLVMType = CContext.GetLLVMType(Type);
        llvm::LoadInst* LoadValue = Builder.CreateAlignedLoad(LLVMType,
            Value, llvm::MaybeAlign(Type->GetAlignment()));
        return CContext.MainArena.Create<IRValue>(LoadValue, Type);
    }

    IRValue *IRBuilder::CreateLoadIfLValue(IRValue *Value)
    {
        if (Value->IsLValue())
            return CreateLoad(Value);
        return Value;
    }

    llvm::StoreInst *IRBuilder::CreateStore(IRValue *Value, llvm::Value *Ptr)
    {
        DataType* Type = Value->GetDataType();
        return Builder.CreateAlignedStore(Value->GetValue(),
            Ptr, llvm::MaybeAlign(Type->GetAlignment()));
    }

    llvm::StoreInst *IRBuilder::CreateStore(IRValue *Value, IRValue *Ptr)
    {
        VoltAssert(Ptr->IsLValue() && "Ptr must be l-value");
        VoltAssert(Value->GetDataType() == Ptr->GetDataType() && "Cannot store different types");
        return CreateStore(Value, Ptr->GetValue());
    }

    IRValue *IRBuilder::CreateNeg(IRValue *Value)
    {
        return CContext.MainArena.Create<IRValue>(Value->GetDataType()->IsFloatingPointType() ?
            Builder.CreateFNeg(Value->GetValue()) :
            Builder.CreateNeg(Value->GetValue()), Value->GetDataType());
    }

    IRValue *IRBuilder::CreateNot(IRValue *Value)
    {
        return CContext.MainArena.Create<IRValue>(
            Builder.CreateNot(Value->GetValue()), Value->GetDataType());
    }

    IRValue *IRBuilder::CreateLogicalNot(IRValue *Value)
    {
        DataType* BoolType = CContext.GetBoolType();
        llvm::Value* BoolValue = Value->CastLLVM(BoolType, Builder, CContext);
        if (!BoolValue) VoltUnreachable("Invalid cast");

        return CContext.MainArena.Create<IRValue>(Builder.CreateNot(BoolValue), BoolType);
    }

    IRValue *IRBuilder::CreateCmp(IRValue *Left, IRValue *Right, OperatorType Op)
    {
        using enum OperatorType;

        DataType* Type = Left->GetDataType();

        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot compare values with different types");

        Arena& MainArena = CContext.MainArena;
        BoolType* BoolTy = CContext.GetBoolType();

        llvm::Type* LLVMType = CContext.GetLLVMType(Type);

        if (auto IntType = Cast<IntegerType>(Type))
        {
            bool IsSigned = IntType->IsSigned();

            llvm::ICmpInst::Predicate Pred;
            switch (Op)
            {
                case Equal:       Pred = llvm::ICmpInst::ICMP_EQ; break;
                case NotEqual:    Pred = llvm::ICmpInst::ICMP_NE; break;
                case Less:        Pred = IsSigned ? llvm::ICmpInst::ICMP_SLT : llvm::ICmpInst::ICMP_ULT; break;
                case LessEqual:   Pred = IsSigned ? llvm::ICmpInst::ICMP_SLE : llvm::ICmpInst::ICMP_ULE; break;
                case Grater:      Pred = IsSigned ? llvm::ICmpInst::ICMP_SGT : llvm::ICmpInst::ICMP_UGT; break;
                case GraterEqual: Pred = IsSigned ? llvm::ICmpInst::ICMP_SGE : llvm::ICmpInst::ICMP_UGE; break;
                default: return nullptr;
            }

            return MainArena.Create<IRValue>(
                Builder.CreateICmp(Pred, Left->GetValue(), Right->GetValue()), BoolTy);
        }

        if (Type->IsFloatingPointType())
        {
            llvm::FCmpInst::Predicate Pred;
            switch (Op)
            {
                case Equal:       Pred = llvm::FCmpInst::FCMP_OEQ; break;
                case NotEqual:    Pred = llvm::FCmpInst::FCMP_ONE; break;
                case Less:        Pred = llvm::FCmpInst::FCMP_OLT; break;
                case LessEqual:   Pred = llvm::FCmpInst::FCMP_OLE; break;
                case Grater:      Pred = llvm::FCmpInst::FCMP_OGT; break;
                case GraterEqual: Pred = llvm::FCmpInst::FCMP_OGE; break;
                default: return nullptr;
            }

            return MainArena.Create<IRValue>(
                Builder.CreateFCmp(Pred, Left->GetValue(), Right->GetValue()), BoolTy);
        }

        return nullptr;
    }

    IRValue *IRBuilder::CreateAdd(IRValue *Left, IRValue *Right)
    {
        Arena& MainArena = CContext.MainArena;

        DataType* LeftType = Left->GetDataType();
        DataType* RightType = Right->GetDataType();

        if (LeftType->IsPointerType() || RightType->IsPointerType())
        {
            IRValue* Ptr = LeftType->IsPointerType() ? Left : Right;
            IRValue* Index = Ptr == Left ? Right : Left;

            if (!Index->GetDataType()->IsIntegerType())
                VoltUnreachable("Cannot add non-integer type to pointer");

            auto PtrType = Cast<PointerType>(Ptr->GetDataType());

            llvm::Type* PointeeType = CContext.GetLLVMType(PtrType->GetBaseType().GetType());
            return MainArena.Create<IRValue>(
                Builder.CreateGEP(PointeeType, Ptr->GetValue(), Index->GetValue()), PtrType);
        }

        if (LeftType != RightType)
            VoltUnreachable("Cannot add values with different types");

        if (LeftType->IsIntegerType())
            return MainArena.Create<IRValue>(
               Builder.CreateAdd(Left->GetValue(), Right->GetValue()), LeftType);

        if (LeftType->IsFloatingPointType())
            return MainArena.Create<IRValue>(
                Builder.CreateFAdd(Left->GetValue(), Right->GetValue()), LeftType);

        VoltUnreachable("Cannot add values with this type");
    }

    IRValue *IRBuilder::CreateSub(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot sub values with different types");

        return CContext.MainArena.Create<IRValue>(Type->IsFloatingPointType() ?
            Builder.CreateFSub(Left->GetValue(), Right->GetValue()) :
            Builder.CreateSub(Left->GetValue(), Right->GetValue()), Type);
    }

    IRValue *IRBuilder::CreateMul(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot multiply values with different types");

        return CContext.MainArena.Create<IRValue>(Type->IsFloatingPointType() ?
            Builder.CreateFMul(Left->GetValue(), Right->GetValue()) :
            Builder.CreateMul(Left->GetValue(), Right->GetValue()), Type);
    }

    IRValue *IRBuilder::CreateDiv(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot multiply values with different types");

        return CContext.MainArena.Create<IRValue>(Type->IsSignedIntegerType() ?
            Builder.CreateSDiv(Left->GetValue(), Right->GetValue()) : Type->IsUnsignedIntegerType() ?
            Builder.CreateUDiv(Left->GetValue(), Right->GetValue()) :
            Builder.CreateFDiv(Left->GetValue(), Right->GetValue()), Type);
    }

    IRValue *IRBuilder::CreateMod(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot apply 'mod' to values with different types");

        if (Type->IsIntegerType())
            return CContext.MainArena.Create<IRValue>(Type->IsSignedIntegerType() ?
                Builder.CreateSRem(Left->GetValue(), Right->GetValue()) :
                Builder.CreateURem(Left->GetValue(), Right->GetValue()), Type);

        VoltUnreachable("Cannot apply 'mod' to non-integer type");
    }

    IRValue *IRBuilder::CreateAnd(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot apply 'and' to values with different types");

        if (Type->IsIntegerType())
            return CContext.MainArena.Create<IRValue>(Builder.CreateAnd(
                Left->GetValue(), Right->GetValue()), Type);

        VoltUnreachable("Cannot apply 'and' to non-integer type");
    }

    IRValue *IRBuilder::CreateOr(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot apply 'or' to values with different types");

        if (Type->IsIntegerType())
            return CContext.MainArena.Create<IRValue>(Builder.CreateOr(
                Left->GetValue(), Right->GetValue()), Type);

        VoltUnreachable("Cannot apply 'or' to non-integer type");
    }

    IRValue *IRBuilder::CreateXor(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot apply 'xor' to values with different types");

        if (Type->IsIntegerType())
            return CContext.MainArena.Create<IRValue>(Builder.CreateXor(
                Left->GetValue(), Right->GetValue()), Type);

        VoltUnreachable("Cannot apply 'xor' to non-integer type");
    }

    IRValue *IRBuilder::CreateRShift(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot apply 'rshift' to values with different types");

        if (auto IntType = Cast<IntegerType>(Type))
            return CContext.MainArena.Create<IRValue>(IntType->IsSigned() ?
                Builder.CreateAShr(Left->GetValue(), Right->GetValue()) :
                Builder.CreateLShr(Left->GetValue(), Right->GetValue()), Type);

        VoltUnreachable("Cannot apply 'rshift' to non-integer type");
    }

    IRValue *IRBuilder::CreateLShift(IRValue *Left, IRValue *Right)
    {
        DataType* Type = Left->GetDataType();
        if (Type != Right->GetDataType())
            VoltUnreachable("Cannot apply 'lshift' to values with different types");

        if (Type->IsIntegerType())
            return CContext.MainArena.Create<IRValue>(Builder.CreateShl(
                Left->GetValue(), Right->GetValue()), Type);

        VoltUnreachable("Cannot apply 'lshift' to non-integer type");
    }

    IRValue *IRBuilder::CreateAssignment(IRValue *Left, IRValue *Right, OperatorType Op)
    {
        using enum OperatorType;

        DataType* Type = Left->GetDataType();
        VoltAssert(Left->IsLValue() && "Cannot apply assignment operator to r-value");
        VoltAssert(Type == Right->GetDataType() && "Cannot assign value with another type");

        if (Op == Assign)
        {
            CreateStore(Right, Left);
            return Right;
        }

        IRValue* NewValue = Left->GetRValue(Builder, CContext);
        switch (Op)
        {
            case AddAssign: NewValue = CreateAdd(NewValue, Right);       break;
            case SubAssign: NewValue = CreateSub(NewValue, Right);       break;
            case MulAssign: NewValue = CreateMul(NewValue, Right);       break;
            case DivAssign: NewValue = CreateDiv(NewValue, Right);       break;
            case ModAssign: NewValue = CreateMod(NewValue, Right);       break;
            case AndAssign: NewValue = CreateAnd(NewValue, Right);       break;
            case OrAssign:  NewValue = CreateOr(NewValue, Right);        break;
            case XorAssign: NewValue = CreateXor(NewValue, Right);       break;
            case LShiftAssign: NewValue = CreateLShift(NewValue, Right); break;
            case RShiftAssign: NewValue = CreateRShift(NewValue, Right); break;
            default: VoltUnreachable("Unknown assignment operator");
        }

        CreateStore(NewValue, Left);
        return NewValue;
    }

    IRValue *IRBuilder::CreateGEP(IRValue *Value, llvm::Value* Index)
    {
        DataType* Type = Value->GetDataType();

        if (auto ArrType = Cast<ArrayType>(Type))
        {
            return CContext.MainArena.Create<IRValue>(
                Builder.CreateGEP(CContext.GetLLVMType(Type),
                Value->GetValue(), { GetInt32(0), Index }),
                ArrType->GetBaseType().GetType(), true);
        }

        if (auto PtrType = Cast<PointerType>(Type))
        {
            return CContext.MainArena.Create<IRValue>(
                Builder.CreateGEP(CContext.GetLLVMType(Type),
                Value->GetValue(), Index), PtrType->GetBaseType().GetType());
        }

        VoltUnreachableFmt("Cannot create GEP to this type: {}", Type->ToString());
    }

}
