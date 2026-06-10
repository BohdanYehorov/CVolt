//
// Created by bohdan on 23.02.26.
//

#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Hash/Hash.h"
#include "Volt/Support/ErrorHandling.h"

namespace Volt
{
	bool DataType::IsSignedIntegerType() const
	{
		if (auto IntType = Cast<const IntegerType>(this))
			return IntType->IsSigned;
		return false;
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

		if (Src->ImplicitCast(Dst))
			return Dst;

		return nullptr;
	}

	bool QualType::CastTo(QualType To, bool Explicit) const
	{
		if (*this == To)
			return true;

		return GetType()->CastTo(To.GetType(), Explicit);
	}

	size_t QualType::GetHash() const
	{
		size_t H = GetType()->GetHash();
		CombineHashes(H, std::hash<size_t>{}(GetQuals()));
		return H;
	}

	bool BoolType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return true;

		if (!Explicit)
			return false;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
			case TypeCategory::CHAR:
			case TypeCategory::INTEGER:
			case TypeCategory::FLOATING_POINT:
				return true;
			default:
				return false;
		}
	}

	bool CharType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return true;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit;
			case TypeCategory::INTEGER:
				return To->IsSignedIntegerType() ? true : Explicit;
			case TypeCategory::CHAR:
			case TypeCategory::FLOATING_POINT:
				return true;
			default:
				return false;
		}
	}

	bool IntegerType::IsEqual(const DataType *Other) const
	{
		if (auto IntType = Cast<const IntegerType>(Other))
			return IntType->BitWidth == BitWidth && IntType->IsSigned == IsSigned;
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
			default: VoltAssert(false);
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
			default: VoltAssert(false);
		}
	}

	std::string IntegerType::ToString() const
	{
		return (IsSigned ? "i" : "u") + std::to_string(BitWidth);
	}

	bool IntegerType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return true;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit;
			case TypeCategory::INTEGER:
				return To->IsSignedIntegerType() == IsSigned ? true : Explicit;
			case TypeCategory::CHAR:
			case TypeCategory::FLOATING_POINT:
				return true;
			default:
				return false;
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
			default: VoltUnreachable("Unsupported FP size");
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
			default:  VoltAssert(false);
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
			default:  VoltAssert(false);
		}
	}

	std::string FloatingPointType::ToString() const
	{
		switch (BitWidth) {
			case 32: return "float";
			case 64: return "double";
			case 128: return "float128";
			default: VoltUnreachable("Unsupported FP size");
		}
	}

	bool FloatingPointType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return true;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit;
			case TypeCategory::INTEGER:
				return To->IsSignedIntegerType() ? true : Explicit;
			case TypeCategory::CHAR:
			case TypeCategory::FLOATING_POINT:
				return true;
			default:
				return false;
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

	bool PointerType::CastTo(DataType *To, bool Explicit) const
	{
		if (this == To)
			return true;

		if (auto PtrType = Cast<PointerType>(To))
		{
			if (BaseType.HasQualifier(QualType::CONST) && !PtrType->BaseType.HasQualifier(QualType::CONST))
				return false;

			if (PtrType->BaseType->GetCategory() == TypeCategory::VOID)
				return true;

			if (Explicit && BaseType->GetCategory() == TypeCategory::VOID)
				return true;

			return false;
		}

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return true;
			default:
				return false;
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

	bool ReferenceType::CastTo(DataType *To, bool Explicit) const
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

	bool ArrayType::CastTo(DataType *To, bool Explicit) const
	{
		if (auto ArrType = Cast<ArrayType>(To))
			if (ArrType->BaseType.GetType() == BaseType.GetType())
				return true;

		return false;
	}
}
