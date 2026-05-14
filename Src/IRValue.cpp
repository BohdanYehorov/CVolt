//
// Created by bohdan on 08.03.26.
//

#include "Volt/Core/Value/IRValue.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include <llvm/Support/ErrorHandling.h>

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
			llvm_unreachable("Type is null");

		if (To->IsReferenceType())
		{
			if (!bIsLValue)
				llvm_unreachable("Cannot bind r-value to reference");

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

	inline llvm::Value* IRValue::CastBooleanTo(DataType *To, llvm::IRBuilder<>& Builder, CompilationContext& CContext)
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
				return Builder.CreateSExt(Value, CContext.GetLLVMType(To));

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

				return FromIntType->BitWidth < ToIntType->BitWidth ?
					Builder.CreateSExt(Value, CContext.GetLLVMType(To)) :
					Builder.CreateTrunc(Value, CContext.GetLLVMType(To));
			}

			case TypeCategory::FLOATING_POINT:
				return Builder.CreateSIToFP(Value, CContext.GetLLVMType(To));

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
			case TypeCategory::INTEGER:
				return Builder.CreateFPToSI(Value, CContext.GetLLVMType(To));

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
