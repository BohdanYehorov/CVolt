//
// Created by bohdan on 23.02.26.
//

#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Hash/Hash.h"

namespace Volt
{
	DataType::~DataType()
	{
		delete[] PointerVariants;
		delete[] ReferenceVariants;
		delete[] ArrayVariants;
	}

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

	bool QualType::CastTo(QualType To, bool Explicit) const
	{
		if (*this == To)
			return true;

		return GetType()->CastTo(To.GetType(), Explicit) != nullptr;
	}

	size_t QualType::GetHash() const
	{
		size_t H = GetType()->GetHash();
		CombineHashes(H, std::hash<size_t>{}(GetQuals()));
		return H;
	}

	DataType *BoolType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return To;

		if (!Explicit)
			return nullptr;

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

	DataType *CharType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return To;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit ? To : nullptr;
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
			default: assert(false);
		}
	}

	size_t IntegerType::GetHash() const
	{
		switch (BitWidth)
		{
			case 8:  return  3;
			case 16: return  4;
			case 32: return  5;
			case 64: return  6;
			default: assert(false);
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

	DataType *IntegerType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return To;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit ? To : nullptr;
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
			default:  assert(false);
		}
	}

	size_t FloatingPointType::GetHash() const
	{
		switch (BitWidth)
		{
			case 16:  return  7;
			case 32:  return  8;
			case 64:  return  9;
			case 128: return 10;
			default:  assert(false);
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

	DataType *FloatingPointType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return To;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit ? To : nullptr;
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
			return BaseType->IsEqual(PtrType->BaseType.GetType());
		return false;
	}

	size_t PointerType::GetHash() const
	{
		size_t Hash = 11;
		CombineHashes(Hash, BaseType->GetHash());
		return Hash;
	}

	DataType *PointerType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return To;

		if (auto PtrType = Cast<PointerType>(To))
		{
			if (BaseType.HasQualifier(QualType::CONST) && !PtrType->BaseType.HasQualifier(QualType::CONST))
				return nullptr;

			if (PtrType->BaseType->GetCategory() == TypeCategory::VOID)
				return To;

			if (Explicit && BaseType->GetCategory() == TypeCategory::VOID)
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
			return BaseType->IsEqual(RefType->BaseType.GetType());
		return false;
	}

	size_t ReferenceType::GetHash() const
	{
		size_t Hash = 12;
		CombineHashes(Hash, BaseType->GetHash());
		return Hash;
	}

	bool ReferenceType::CanBind(QualType Type) const
	{
		if (Type.GetType() == this)
			return true;

		if (!BaseType.HasQualifier(QualType::CONST) &&
			Type.HasQualifier(QualType::CONST))
			return false;

		return Type.GetType() == BaseType.GetType();
	}

	DataType *ReferenceType::CastTo(DataType *To, bool Explicit) const
	{
		return BaseType->CastTo(To, Explicit);
	}

	bool ArrayType::IsEqual(const DataType *Other) const
	{
		if (!LengthInit)
			return false;

		if (auto ArrType = Cast<const ArrayType>(Other))
			return BaseType->IsEqual(ArrType->BaseType.GetType()) && Length == ArrType->Length;
		return false;
	}

	size_t ArrayType::GetHash() const
	{
		size_t Hash = 13;
		CombineHashes(Hash, BaseType->GetHash());
		CombineHashes(Hash, Length);
		return Hash;
	}

	std::string ArrayType::ToString() const
	{
		return BaseType ? BaseType->ToString() + "[" + std::to_string(Length) + "]" : "?";
	}

	DataType *ArrayType::CastTo(DataType *To, bool Explicit) const
	{
		if (auto ArrType = Cast<ArrayType>(To))
			if (ArrType->BaseType.GetType() == BaseType.GetType())
				return To;

		return nullptr;
	}
}
