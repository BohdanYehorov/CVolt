//
// Created by bohdan on 23.02.26.
//

#include "Volt/Core/Types/DataType.h"

namespace Volt
{
	bool IntegerType::IsEqual(const DataType *Other) const
	{
		if (auto IntType = Cast<const IntegerType>(Other))
			return IntType->BitWidth == BitWidth;
		return false;
	}

	int IntegerType::GetRank() const
	{
		switch (BitWidth)
		{
			case 8:  return  3;
			case 16: return  4;
			case 32: return  5;
			case 64: return  6;
			default: return -1;
		}
	}

	std::string IntegerType::ToString() const
	{
		switch (BitWidth)
		{
			case 8:  return "byte";
			case 32: return "int";
			case 64: return "long";
			default: throw std::runtime_error("Unsupported integer size");
		}
	}

	bool FloatingPointType::IsEqual(const DataType *Other) const
	{
		if (auto FloatType = Cast<const FloatingPointType>(Other))
			return FloatType->BitWidth == BitWidth;
		return false;
	}

	llvm::Type *FloatingPointType::ToLLVMType(llvm::LLVMContext &Context) const
	{
		switch (BitWidth)
		{
			case 16: return llvm::Type::getHalfTy(Context);
			case 32: return llvm::Type::getFloatTy(Context);
			case 64: return llvm::Type::getDoubleTy(Context);
			case 128: return llvm::Type::getFP128Ty(Context);
			default: throw std::runtime_error("Unsupported FP size");
		}
	}

	int FloatingPointType::GetRank() const
	{
		switch (BitWidth)
		{
			case 16:  return  7;
			case 32:  return  8;
			case 64:  return  9;
			case 128: return 10;
			default:  return -1;
		}
	}

	std::string FloatingPointType::ToString() const
	{
		switch (BitWidth) {
			case 32: return "float";
			case 64: return "double";
			case 128: return "float128";
			default: throw std::runtime_error("Unsupported FP size");
		}
	}

	bool PointerType::IsEqual(const DataType *Other) const
	{
		if (auto PtrType = Cast<const PointerType>(Other))
			return BaseType->IsEqual(PtrType->BaseType);
		return false;
	}

	bool ReferenceType::IsEqual(const DataType *Other) const
	{
		if (auto RefType = Cast<const ReferenceType>(Other))
			return BaseType->IsEqual(RefType->BaseType);
		return false;
	}

	bool ArrayType::IsEqual(const DataType *Other) const
	{
		if (!LengthInit)
			return false;

		if (auto ArrType = Cast<const ArrayType>(Other))
			return BaseType->IsEqual(ArrType->BaseType) && Length == ArrType->Length;
		return false;
	}

	std::string ArrayType::ToString() const
	{
		return BaseType ? BaseType->ToString() + "[" + std::to_string(Length) + "]" : "?";
	}

	bool ConstType::IsEqual(const DataType *Other) const
	{
		if (auto CstType = Cast<const ConstType>(Other))
			return BaseType->IsEqual(CstType->BaseType);
		return false;
	}
}
