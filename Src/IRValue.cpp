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

	bool IRValue::CastTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
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
				return CastPointerTo(To, Builder);
			case TypeCategory::REFERENCE:
				return CastReferenceTo(To, Builder, CContext);
			default:
				return false;
		}
	}

	bool IRValue::ToRValue(llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (!bIsLValue)
			return false;

		llvm::Type* LLVMType = CContext.GetLLVMType(Type);
		Value = Builder.CreateLoad(LLVMType, Value);
		bIsLValue = false;
		return true;
	}

	IRValue* IRValue::GetRValue(llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (!bIsLValue)
			return this;

		llvm::Type* LLVMType = CContext.GetLLVMType(Type);
		return CContext.MainArena.Create<IRValue>(Builder.CreateLoad(LLVMType, Value), Type);
	}

	inline bool IRValue::CastBooleanTo(DataType *To, llvm::IRBuilder<>& Builder, CompilationContext& CContext)
	{
		if (Type == To)
			return true;

		switch (To->GetCategory())
		{
			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
				Value = Builder.CreateZExt(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;

			case TypeCategory::FLOATING_POINT:
				Value = Builder.CreateSIToFP(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;
			default:
				return false;
		}
	}

	bool IRValue::CastCharTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To)
			return true;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				Value = Builder.CreateICmpNE(Value, Builder.getInt8(0));
				Type = To;
				return true;

			case TypeCategory::INTEGER:
				Value = Builder.CreateSExt(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;

			case TypeCategory::FLOATING_POINT:
				Value = Builder.CreateSIToFP(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;

			default:
				return false;
		}
	}

	bool IRValue::CastIntegerTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To)
			return true;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				Value = Builder.CreateICmpNE(Value,
					llvm::ConstantInt::get(CContext.GetLLVMType(To), 0));
				Type = To;
				return true;

			case TypeCategory::CHAR:
				Value = Builder.CreateTrunc(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;

			case TypeCategory::INTEGER:
			{
				auto FromIntType = Cast<IntegerType>(Type);
				auto ToIntType = Cast<IntegerType>(To);

				if (!FromIntType || !ToIntType)
					return false;

				Value = FromIntType->BitWidth < ToIntType->BitWidth ?
					Builder.CreateSExt(Value, CContext.GetLLVMType(To)) :
					Builder.CreateTrunc(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;
			}

			case TypeCategory::FLOATING_POINT:
				Value = Builder.CreateSIToFP(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;

			case TypeCategory::REFERENCE:
				Type = To;
				return true;

			default:
				return false;
		}
	}

	bool IRValue::CastFloatTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (Type == To)
			return true;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				Value = Builder.CreateFCmpONE(Value,
					llvm::ConstantFP::get(CContext.GetLLVMType(To), 0.0));
				Type = To;
				return true;

			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
				Value = Builder.CreateFPToSI(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;

			case TypeCategory::FLOATING_POINT:
			{
				auto FromFloatType = Cast<FloatingPointType>(Type);
				auto ToFloatType = Cast<FloatingPointType>(To);

				if (!FromFloatType || !ToFloatType)
					return false;

				Value = FromFloatType->BitWidth < ToFloatType->BitWidth ?
					Builder.CreateFPExt(Value, CContext.GetLLVMType(To)) :
					Builder.CreateFPTrunc(Value, CContext.GetLLVMType(To));
				Type = To;
				return true;
			}

			default:
				return false;
		}
	}

	bool IRValue::CastPointerTo(DataType *To, llvm::IRBuilder<> &Builder)
	{
		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				Value = Builder.CreateICmpNE(
					Value, llvm::ConstantPointerNull::get(Builder.getPtrTy(0)));
				Type = To;
				return true;
			case TypeCategory::POINTER:
				Type = To;
				return true;
			default:
				return false;
		}
	}

	bool IRValue::CastReferenceTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (auto RefType = Cast<ReferenceType>(Type))
		{
			if (!RefType->BaseType->CastTo(To, true))
				return false;

			Value = Builder.CreateLoad(CContext.GetLLVMType(RefType->BaseType.GetType()), Value);
			Type = RefType->BaseType.GetType();
			if (!CastTo(To, Builder, CContext))
				llvm_unreachable("Cast failed");

			return true;
		}

		return false;
	}
}
