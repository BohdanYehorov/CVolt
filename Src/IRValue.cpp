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
			Type = RefType->GetBaseType().GetType();
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

	llvm::Value* IRValue::CastLLVM(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		switch (Type->GetCategory())
		{
			case TypeCategory::Boolean:
				return CastBooleanTo(To, Builder, CContext);
			case TypeCategory::Char:
				return CastCharTo(To, Builder, CContext);
			case TypeCategory::Integer:
				return CastIntegerTo(To, Builder, CContext);
			case TypeCategory::FloatingPoint:
				return CastFloatTo(To, Builder, CContext);
			case TypeCategory::Pointer:
				return CastPointerTo(To, Builder, CContext);
			case TypeCategory::NullPointer:
				return CastNullPointerTo(To, Builder, CContext);
			case TypeCategory::Reference:
				return CastReferenceTo(To, Builder, CContext);
			default:
				return nullptr;
		}
	}

	llvm::Value* IRValue::CastBooleanTo(DataType *To, llvm::IRBuilder<>& Builder, CompilationContext& CContext)
	{
		if (Type == To)
			return Value;

		switch (To->GetCategory())
		{
			case TypeCategory::Char:
			case TypeCategory::Integer:
				return Builder.CreateZExt(Value, CContext.GetLLVMType(To));

			case TypeCategory::FloatingPoint:
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
			case TypeCategory::Boolean:
				return Builder.CreateICmpNE(Value, Builder.getInt8(0));

			case TypeCategory::Integer:
				return To->IsSignedIntegerType() ? Builder.CreateSExt(Value, CContext.GetLLVMType(To)) :
				                                   Builder.CreateZExt(Value, CContext.GetLLVMType(To));

			case TypeCategory::FloatingPoint:
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
			case TypeCategory::Boolean:
				return Builder.CreateICmpNE(Value,
				llvm::ConstantInt::get(CContext.GetLLVMType(To), 0));

			case TypeCategory::Char:
				return Builder.CreateTrunc(Value, CContext.GetLLVMType(To));

			case TypeCategory::Integer:
			{
				auto FromIntType = Cast<IntegerType>(Type);
				auto ToIntType = Cast<IntegerType>(To);

				if (!FromIntType || !ToIntType)
					return nullptr;

				return FromIntType->GetBitWidth() < ToIntType->GetBitWidth() ? ToIntType->IsSigned() ?
					Builder.CreateSExt(Value, CContext.GetLLVMType(To))   :
					Builder.CreateZExt(Value, CContext.GetLLVMType(Type)) :
					Builder.CreateTrunc(Value, CContext.GetLLVMType(To));
			}

			case TypeCategory::FloatingPoint:
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
			case TypeCategory::Boolean:
				return Builder.CreateFCmpONE(Value,
				 	llvm::ConstantFP::get(CContext.GetLLVMType(To), 0.0));

			case TypeCategory::Char:
				return Builder.CreateFPToSI(Value, CContext.GetLLVMType(To));

			case TypeCategory::Integer:
				return To->IsSignedIntegerType() ? Builder.CreateFPToSI(Value, CContext.GetLLVMType(To)) :
				                                   Builder.CreateFPToUI(Value, CContext.GetLLVMType(To));

			case TypeCategory::FloatingPoint:
			{
				auto FromFloatType = Cast<FloatingPointType>(Type);
				auto ToFloatType = Cast<FloatingPointType>(To);

				if (!FromFloatType || !ToFloatType)
					return nullptr;

				return FromFloatType->GetBitWidth() < ToFloatType->GetBitWidth() ?
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
			case TypeCategory::Boolean:
				return Builder.CreateICmpNE(
					Value, llvm::ConstantPointerNull::get(Builder.getPtrTy(0)));
			case TypeCategory::Pointer:
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
			if (!RefType->GetBaseType()->CastTo(To, true))
				return nullptr;

			auto Val = MainArena.Create<IRValue>(Builder.CreateLoad(
				CContext.GetLLVMType(RefType->GetBaseType().GetType()), Value), RefType->GetBaseType().GetType());

			return Val->CastLLVM(To, Builder, CContext);
		}

		return nullptr;
	}
}
