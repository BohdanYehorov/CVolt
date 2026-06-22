//
// Created by bohdan on 23.02.26.
//

#include "Volt/Core/Types/DataType.h"

#include "Volt/Core/TypeDefs/TypeDefs.h"
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

	QualType QualType::GetNotReferenceType() const
	{
		if (auto RefType = CastAs<ReferenceType>())
			return RefType->BaseType;
		return *this;
	}

	std::string QualType::ToString() const
	{
		std::string Quals;
		if (HasQualifier(CONST))
			Quals += "const ";
		return Quals + GetType()->ToString();
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

	llvm::Type *FloatingPointType::ToLLVMType(llvm::LLVMContext &Context) const
	{
		switch (BitWidth)
		{
			case 16: return llvm::Type::getHalfTy(Context);
			case 32: return llvm::Type::getFloatTy(Context);
			case 64: return llvm::Type::getDoubleTy(Context);
			case 128: return llvm::Type::getFP128Ty(Context);
			default: VoltUnreachable("Invalid floating point bit width");
		}
	}

	std::string FloatingPointType::ToString() const
	{
		return "f" + std::to_string(BitWidth);
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

	std::string PointerType::ToString() const
	{
		if (BaseType.GetQuals() == 0)
			return BaseType->ToString() + "*";
		return "(" + BaseType.ToString() + ")*";
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

	std::string ArrayType::ToString() const
	{
		if (BaseType.GetQuals() == 0)
			return BaseType->ToString() + "[" + std::to_string(Length) + "]";
		return "(" + BaseType.ToString() + ")[" + std::to_string(Length) + "]";
	}

	bool ArrayType::CastTo(DataType *To, bool Explicit) const
	{
		if (auto ArrType = Cast<ArrayType>(To))
			if (ArrType->BaseType.GetType() == BaseType.GetType())
				return true;

		return false;
	}
}
