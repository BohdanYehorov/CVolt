//
// Created by bohdan on 08.03.26.
//

#include "Volt/Core/Value/TypedValue.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"

namespace Volt
{
	bool TypedValue::CastTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
	{
		if (auto ConstTy = Cast<ConstType>(To))
			return CastTo(ConstTy->BaseType, Builder, CContext);

		if (auto ConstTy = Cast<ConstType>(Type))
		{
			Type = ConstTy->BaseType;
			if (!CastTo(To, Builder, CContext))
			{
				Type = ConstTy;
				return false;
			}

			return true;
		}

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
			default:
				return false;
		}
	}

	inline bool TypedValue::CastBooleanTo(DataType *To, llvm::IRBuilder<>& Builder, CompilationContext& CContext)
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

	bool TypedValue::CastCharTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
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

	bool TypedValue::CastIntegerTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
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

			default:
				return false;
		}
	}

	bool TypedValue::CastFloatTo(DataType *To, llvm::IRBuilder<> &Builder, CompilationContext &CContext)
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

	bool TypedValue::CastPointerTo(DataType *To, llvm::IRBuilder<> &Builder)
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
}
