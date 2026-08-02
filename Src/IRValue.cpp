//
// Created by bohdan on 08.03.26.
//

#include "Volt/Compiler/Value/IRValue.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Support/ErrorHandling.h"

namespace Volt
{
	IRValue::IRValue(llvm::Value *InValue, DataType *InType, llvm::IRBuilder<> &Builder) : bIsLValue(true)
	{
		if (auto RefType = Cast<ReferenceType>(InType))
		{
			Value = InValue;
			Type = RefType->BaseType.GetType();
			return;
		}

		Value = Builder.CreateAlloca(
			InValue->getType(), nullptr, InValue->getName());

		Builder.CreateStore(InValue, Value);
		Type = InType;
	}

	IRValue* IRValue::CastTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (To == Type)
			return this;

		if (llvm::Value* Val = CastLLVM(To, Builder, CContext))
			return CContext.MainArena.Create<IRValue>(Val, To);

		return nullptr;
	}

	IRValue* IRValue::CastOrBind(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (!To)
			VoltUnreachable("Type is null");

		if (To->IsReferenceType())
		{
			if (!bIsLValue)
				VoltUnreachable("Cannot bind r-value to reference");

			return this;
		}

		return GetRValue(Builder, CContext)->CastTo(To, Builder, CContext);
	}

	llvm::Value* IRValue::CastLLVM(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		switch (Type->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return CastBooleanTo(To, Builder, CContext);
			case TypeCategory::CHAR:
				return CastCharTo(To, Builder, CContext);
			case TypeCategory::INTEGER:
				return CastIntegerTo(To, Builder, CContext);
			case TypeCategory::FLOATING_POINT:
				return CastFloatTo(To, Builder, CContext);
			case TypeCategory::POINTER:
				return CastPointerTo(To, Builder, CContext);
			case TypeCategory::NULL_POINTER:
				return CastNullPointerTo(To, Builder, CContext);
			case TypeCategory::REFERENCE:
				return CastReferenceTo(To, Builder, CContext);
			default:
				return nullptr;
		}
	}

	IRValue* IRValue::GetRValue(llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (!bIsLValue)
			return this;

		llvm::Type* LLVMType = CContext.GetLLVMType(Type);
		return CContext.MainArena.Create<IRValue>(Builder.CreateLoad(LLVMType, Value), Type);
	}

	IRValue* IRValue::CreateNeg(llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		return CContext.MainArena.Create<IRValue>(Type->IsFloatingPointType() ?
			Builder.CreateFNeg(Value) : Builder.CreateNeg(Value), Type);
	}

	IRValue* IRValue::CreateNot(llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		return CContext.MainArena.Create<IRValue>(Builder.CreateNot(Value), Type);
	}

	IRValue* IRValue::CreateLogicalNot(llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		DataType* BoolType = CContext.GetBoolType();
		llvm::Value* BoolValue = CastLLVM(BoolType, Builder, CContext);
		if (!BoolValue)
			VoltUnreachable("Invalid cast");

		return CContext.MainArena.Create<IRValue>(Builder.CreateNot(BoolValue), BoolType);
	}

	IRValue* IRValue::CreateCmp(IRValue *Right, OperatorType Op, llvm::IRBuilder<> &Builder, CompilationContext& CContext)
	{
		using enum OperatorType;

		if (Type != Right->Type)
			VoltUnreachable("Cannot compare values with different types");

		Arena& MainArena = CContext.MainArena;

		BoolType* BoolTy = CContext.GetBoolType();

		if (auto IntType = Cast<IntegerType>(Type))
		{
			bool IsSigned = IntType->IsSigned;

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
				Builder.CreateICmp(Pred, Value, Right->Value), BoolTy);
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
				Builder.CreateFCmp(Pred, Value, Right->GetValue()), BoolTy);
		}

		return nullptr;
	}

	IRValue* IRValue::CreateAdd(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		Arena& MainArena = CContext.MainArena;

		DataType* RightType = Right->Type;

		if (Type->IsPointerType() || RightType->IsPointerType())
		{
			IRValue* Ptr = Type->IsPointerType() ? this : Right;
			IRValue* Index = Ptr == this ? Right : this;

			if (!Index->Type->IsIntegerType())
				VoltUnreachable("Cannot add non-integer type to pointer");

			auto PtrType = Cast<PointerType>(Ptr->Type);

			llvm::Type* PointeeType = CContext.GetLLVMType(PtrType->BaseType.GetType());
			return MainArena.Create<IRValue>(
				Builder.CreateGEP(PointeeType, Ptr->Value, Index->Value), PtrType);
		}

		if (Type != Right->Type)
			VoltUnreachable("Cannot add values with different types");

		if (Type->IsIntegerType())
			return MainArena.Create<IRValue>(
			   Builder.CreateAdd(Value, Right->Value), Type);

		if (Type->IsFloatingPointType())
			return MainArena.Create<IRValue>(
				Builder.CreateFAdd(Value, Right->Value), Type);

		VoltUnreachable("Cannot add values with this type");
	}

	IRValue* IRValue::CreateSub(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot sub values with different types");

		return CContext.MainArena.Create<IRValue>(Type->IsFloatingPointType() ?
			Builder.CreateFSub(Value, Right->Value) :
			Builder.CreateSub(Value, Right->Value), Type);
	}

	IRValue* IRValue::CreateMul(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot multiply values with different types");

		return CContext.MainArena.Create<IRValue>(Type->IsFloatingPointType() ?
			Builder.CreateFMul(Value, Right->Value) :
			Builder.CreateMul(Value, Right->Value), Type);
	}

	IRValue* IRValue::CreateDiv(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot multiply values with different types");

		if (auto IntType = Cast<IntegerType>(Type))
			return CContext.MainArena.Create<IRValue>(IntType->IsSigned ?
				Builder.CreateSDiv(Value, Right->Value) :
				Builder.CreateUDiv(Value, Right->Value), Type);

		return CContext.MainArena.Create<IRValue>(Builder.CreateFDiv(Value, Right->Value), Type);
	}

	IRValue* IRValue::CreateMod(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot apply 'mod' to values with different types");

		if (auto IntType = Cast<IntegerType>(Type))
			return CContext.MainArena.Create<IRValue>(IntType->IsSigned ?
				Builder.CreateSRem(Value, Right->Value) :
				Builder.CreateURem(Value, Right->Value), Type);

		VoltUnreachable("Cannot apply 'mod' to non-integer type");
	}

	IRValue* IRValue::CreateBitAnd(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot apply 'and' to values with different types");

		if (Type->IsIntegerType())
			return CContext.MainArena.Create<IRValue>(Builder.CreateAnd(Value, Right->Value), Type);

		VoltUnreachable("Cannot apply 'and' to non-integer type");
	}

	IRValue* IRValue::CreateBitOr(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot apply 'or' to values with different types");

		if (Type->IsIntegerType())
			return CContext.MainArena.Create<IRValue>(Builder.CreateOr(Value, Right->Value), Type);

		VoltUnreachable("Cannot apply 'or' to non-integer type");
	}

	IRValue* IRValue::CreateBitXor(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot apply 'xor' to values with different types");

		if (Type->IsIntegerType())
			return CContext.MainArena.Create<IRValue>(Builder.CreateXor(Value, Right->Value), Type);

		VoltUnreachable("Cannot apply 'xor' to non-integer type");
	}

	IRValue* IRValue::CreateRShift(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot apply 'rshift' to values with different types");

		if (auto IntType = Cast<IntegerType>(Type))
			return CContext.MainArena.Create<IRValue>(IntType->IsSigned ?
				Builder.CreateAShr(Value, Right->Value) :
				Builder.CreateLShr(Value, Right->Value), Type);

		VoltUnreachable("Cannot apply 'rshift' to non-integer type");
	}

	IRValue* IRValue::CreateLShift(IRValue *Right, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type != Right->Type)
			VoltUnreachable("Cannot apply 'lshift' to values with different types");

		if (Type->IsIntegerType())
			return CContext.MainArena.Create<IRValue>(Builder.CreateShl(Value, Right->Value), Type);

		VoltUnreachable("Cannot apply 'lshift' to non-integer type");
	}

	IRValue* IRValue::CreateAssignment(IRValue *Right, OperatorType Op,
	                                   llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		using enum OperatorType;

		if (!bIsLValue)
			VoltUnreachable("Cannot apply assignment operator to r-value");

		if (Type != Right->Type)
			VoltUnreachable("Cannot assign value with another type");

		if (Op == Assign)
		{
			Builder.CreateStore(Right->Value, Value);
			return Right;
		}

		IRValue* NewValue = GetRValue(Builder, CContext);
		switch (Op)
		{
			case AddAssign:
				NewValue = NewValue->CreateAdd(Right, Builder, CContext);
				break;

			case SubAssign:
				NewValue = NewValue->CreateSub(Right, Builder, CContext);
				break;

			case MulAssign:
				NewValue = NewValue->CreateMul(Right, Builder, CContext);
				break;

			case DivAssign:
				NewValue = NewValue->CreateDiv(Right, Builder, CContext);
				break;

			case ModAssign:
				NewValue = NewValue->CreateMod(Right, Builder, CContext);
				break;

			case AndAssign:
				NewValue = NewValue->CreateBitAnd(Right, Builder, CContext);
				break;

			case OrAssign:
				NewValue = NewValue->CreateBitOr(Right, Builder, CContext);
				break;

			case XorAssign:
				NewValue = NewValue->CreateBitXor(Right, Builder, CContext);
				break;

			case RShiftAssign:
				NewValue = NewValue->CreateRShift(Right, Builder, CContext);
				break;

			case LShiftAssign:
				NewValue = NewValue->CreateLShift(Right, Builder, CContext);
				break;

			default:
				VoltUnreachable("Unknown assignment operator");
		}

		Builder.CreateStore(NewValue->Value, Value);
		return NewValue;
	}

	llvm::Value* IRValue::CastBooleanTo(DataType *To, llvm::IRBuilder<>& Builder, CompilationContext& CContext)
	{
		if (Type == To)
			return Value;

		switch (To->GetCategory())
		{
			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
				return Builder.CreateZExt(Value, CContext.GetLLVMType(To));

			case TypeCategory::FLOATING_POINT:
				return Builder.CreateSIToFP(Value, CContext.GetLLVMType(To));
			default:
				return nullptr;
		}
	}

	llvm::Value* IRValue::CastCharTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To)
			return Value;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Builder.CreateICmpNE(Value, Builder.getInt8(0));

			case TypeCategory::INTEGER:
				return To->IsSignedIntegerType() ? Builder.CreateSExt(Value, CContext.GetLLVMType(To)) :
				                                   Builder.CreateZExt(Value, CContext.GetLLVMType(To));

			case TypeCategory::FLOATING_POINT:
				return Builder.CreateSIToFP(Value, CContext.GetLLVMType(To));
			default:
				return nullptr;
		}
	}

	llvm::Value* IRValue::CastIntegerTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To)
			return Value;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Builder.CreateICmpNE(Value,
				llvm::ConstantInt::get(CContext.GetLLVMType(To), 0));

			case TypeCategory::CHAR:
				return Builder.CreateTrunc(Value, CContext.GetLLVMType(To));

			case TypeCategory::INTEGER:
			{
				auto FromIntType = Cast<IntegerType>(Type);
				auto ToIntType = Cast<IntegerType>(To);

				if (!FromIntType || !ToIntType)
					return nullptr;

				return FromIntType->BitWidth < ToIntType->BitWidth ? ToIntType->IsSigned ?
					Builder.CreateSExt(Value, CContext.GetLLVMType(To))   :
					Builder.CreateZExt(Value, CContext.GetLLVMType(Type)) :
					Builder.CreateTrunc(Value, CContext.GetLLVMType(To));
			}

			case TypeCategory::FLOATING_POINT:
				return Type->IsSignedIntegerType() ? Builder.CreateSIToFP(Value, CContext.GetLLVMType(To)) :
													 Builder.CreateUIToFP(Value, CContext.GetLLVMType(To));

			default:
				return nullptr;
		}
	}

	llvm::Value* IRValue::CastFloatTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To)
			return Value;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Builder.CreateFCmpONE(Value,
				 	llvm::ConstantFP::get(CContext.GetLLVMType(To), 0.0));

			case TypeCategory::CHAR:
				return Builder.CreateFPToSI(Value, CContext.GetLLVMType(To));

			case TypeCategory::INTEGER:
				return To->IsSignedIntegerType() ? Builder.CreateFPToSI(Value, CContext.GetLLVMType(To)) :
				                                   Builder.CreateFPToUI(Value, CContext.GetLLVMType(To));

			case TypeCategory::FLOATING_POINT:
			{
				auto FromFloatType = Cast<FloatingPointType>(Type);
				auto ToFloatType = Cast<FloatingPointType>(To);

				if (!FromFloatType || !ToFloatType)
					return nullptr;

				return FromFloatType->BitWidth < ToFloatType->BitWidth ?
					Builder.CreateFPExt(Value, CContext.GetLLVMType(To)) :
					Builder.CreateFPTrunc(Value, CContext.GetLLVMType(To));
			}

			default:
				return nullptr;
		}
	}

	llvm::Value* IRValue::CastPointerTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext& CContext)
	{
		if (To == Type)
			return Value;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Builder.CreateICmpNE(
					Value, llvm::ConstantPointerNull::get(Builder.getPtrTy(0)));
			case TypeCategory::POINTER:
				return Value;
			default:
				return nullptr;
		}
	}

	llvm::Value *IRValue::CastNullPointerTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To || To->IsPointerType())
			return Value;

		return nullptr;
	}

	llvm::Value* IRValue::CastReferenceTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To)
			return Value;

		Arena& MainArena = CContext.MainArena;

		if (auto RefType = Cast<ReferenceType>(Type))
		{
			if (!RefType->BaseType->CastTo(To, true))
				return nullptr;

			auto Val = MainArena.Create<IRValue>(Builder.CreateLoad(
				CContext.GetLLVMType(RefType->BaseType.GetType()), Value), RefType->BaseType.GetType());

			return Val->CastLLVM(To, Builder, CContext);
		}

		return nullptr;
	}
}
