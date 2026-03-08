//
// Created by bohdan on 23.02.26.
//

#include "Volt/Core/Types/DataType.h"

namespace Volt
{
	DataType *DataType::GetJointType(DataType *Left, DataType *Right)
	{
		if (Left == Right)
			return Left;

		int LeftTypeRank = Left->GetRank();
		int RightTypeRank = Right->GetRank();

		if (LeftTypeRank == -1 || RightTypeRank == -1)
			return nullptr;

		if (LeftTypeRank == RightTypeRank)
			return Left;

		DataType* Src = LeftTypeRank > RightTypeRank ? Right : Left;
		DataType* Dst = LeftTypeRank > RightTypeRank ? Left : Right;

		return Src->ImplicitCast(Dst);
	}

	DataType *BoolType::ImplicitCast(DataType *To) const
	{
		if (this == To)
			return To;

		if (auto CstType = Cast<ConstType>(To))
		{
			if (ImplicitCast(CstType->BaseType))
				return To;
			return nullptr;
		}

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
				return To;
			default:
				return nullptr;
		}
	}

	DataType *CharType::ImplicitCast(DataType *To) const
	{
		if (this == To)
			return To;

		if (auto CstType = Cast<ConstType>(To))
		{
			if (ImplicitCast(CstType->BaseType))
				return To;
			return nullptr;
		}

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
			case TypeCategory::FLOATING_POINT:
				return To;
			default:
				return nullptr;
		}
	}

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

	DataType *IntegerType::ImplicitCast(DataType *To) const
	{
		if (this == To)
			return To;

		if (auto CstType = Cast<ConstType>(To))
		{
			if (ImplicitCast(CstType->BaseType))
				return To;
			return nullptr;
		}

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
			case TypeCategory::FLOATING_POINT:
				return To;
			default:
				return nullptr;
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

	DataType *FloatingPointType::ImplicitCast(DataType *To) const
	{
		if (this == To)
			return To;

		if (auto CstType = Cast<ConstType>(To))
		{
			if (ImplicitCast(CstType->BaseType))
				return To;
			return nullptr;
		}

		switch (To->GetCategory())
		{
			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
			case TypeCategory::FLOATING_POINT:
				return To;
			default:
				return nullptr;
		}
	}

	bool PointerType::IsEqual(const DataType *Other) const
	{
		if (auto PtrType = Cast<const PointerType>(Other))
			return BaseType->IsEqual(PtrType->BaseType);
		return false;
	}

	DataType *PointerType::ImplicitCast(DataType *To) const
	{
		if (this == To)
			return To;

		if (auto CstType = Cast<ConstType>(To))
		{
			if (ImplicitCast(CstType->BaseType))
				return To;
			return nullptr;
		}

		if (auto PtrType = Cast<PointerType>(To))
		{
			if (PtrType->BaseType->GetCategory() == TypeCategory::VOID)
				return To;
			return nullptr;
		}

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return To;
			default:
				return nullptr;
		}
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

	DataType *ArrayType::ImplicitCast(DataType *To) const
	{
		if (this == To)
			return To;

		if (auto CstType = Cast<ConstType>(To))
		{
			if (ImplicitCast(CstType->BaseType))
				return To;
			return nullptr;
		}

		return nullptr;
	}

	bool ConstType::IsEqual(const DataType *Other) const
	{
		if (auto CstType = Cast<const ConstType>(Other))
			return BaseType->IsEqual(CstType->BaseType);
		return false;
	}

	DataType* ConstType::ImplicitCast(DataType *To) const
	{
		if (this == To)
			return To;

		return nullptr;
	}
}
