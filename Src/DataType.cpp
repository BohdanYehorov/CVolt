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
			return IntType->IsSigned();
		return false;
	}

	llvm::Type *DataType::GetLLVMOrCachedType(llvm::LLVMContext &Context)
	{
		if (!CachedType)
			CachedType = ToLLVMType(Context);
		return CachedType;
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

	CastKind QualType::CastTo(QualType To) const
	{
		if (*this == To)
			return CastKind::Exact;

		return GetType()->CastTo(To.GetType());
	}

	QualType QualType::GetNotReferenceType() const
	{
		if (auto RefType = CastAs<ReferenceType>())
			return RefType->GetBaseType();
		return *this;
	}

	std::string QualType::ToString() const
	{
		std::string Quals;
		if (HasQualifier(CONST))
			Quals += "const ";
		return Quals + GetType()->ToString();
	}

	std::string QualType::GetIRName() const
	{
		return HasQualifier(CONST) ? "C" + GetType()->GetIRName() : GetType()->GetIRName();
	}

	CastKind BoolType::CastTo(DataType *To) const
	{
		if (this == To)
			return CastKind::Exact;

		switch (To->GetCategory())
		{
			case TypeCategory::Char:
			case TypeCategory::Integer:
			case TypeCategory::FloatingPoint:
				return CastKind::Explicit;
			default:
				return CastKind::Invalid;
		}
	}

	CastKind CharType::CastTo(DataType *To) const
	{
		if (this == To)
			return CastKind::Exact;

		switch (To->GetCategory())
		{
			case TypeCategory::Boolean:
			case TypeCategory::Integer:
			case TypeCategory::FloatingPoint:
				return CastKind::Explicit;
			default:
				return CastKind::Invalid;
		}
	}

	std::string IntegerType::ToString() const
	{
		return (bIsSigned ? "i" : "u") + std::to_string(BitWidth);
	}

	std::string IntegerType::GetIRName() const
	{
		if (bIsSigned)
		{
			switch (BitWidth)
			{
				case 8:  return "k";
				case 16: return "s";
				case 32: return "i";
				case 64: return "l";
				default: VoltUnreachableFmt("Invalid integer bit width: {}", BitWidth);
			}
		}

		switch (BitWidth)
		{
			case 8:  return "r";
			case 16: return "t";
			case 32: return "u";
			case 64: return "m";
			default: VoltUnreachableFmt("Invalid integer bit width: {}", BitWidth);
		}
	}

	CastKind IntegerType::CastTo(DataType *To) const
	{
		if (this == To)
			return CastKind::Exact;

		switch (To->GetCategory())
		{
			case TypeCategory::Boolean:
			case TypeCategory::Char:
				return CastKind::Explicit;
			case TypeCategory::Integer:
				if (To->IsSignedIntegerType() == bIsSigned)
					return To->GetSize() > GetSize() ? CastKind::Ext : CastKind::Trunc;
				return CastKind::Explicit;
			case TypeCategory::FloatingPoint:
				return CastKind::CategoryConv;
			default:
				return CastKind::Invalid;
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
			default: VoltUnreachableFmt("Invalid floating point bit width: {}", BitWidth);
		}
	}

	std::string FloatingPointType::ToString() const
	{
		return "f" + std::to_string(BitWidth);
	}

	std::string FloatingPointType::GetIRName() const
	{
		switch (BitWidth)
		{
			case 16: return "h";
			case 32: return "f";
			case 64: return "d";
			case 128: return "q";
			default: VoltUnreachableFmt("Invalid floating point bit width: {}", BitWidth);
		}
	}

	CastKind FloatingPointType::CastTo(DataType *To) const
	{
		if (this == To)
			return CastKind::Exact;

		switch (To->GetCategory())
		{
			case TypeCategory::Boolean:
			case TypeCategory::Char:
				return CastKind::Explicit;
			case TypeCategory::Integer:
				return To->IsSignedIntegerType() ? CastKind::CategoryConv :
												   CastKind::Explicit;
			case TypeCategory::FloatingPoint:
				return To->GetSize() > GetSize() ? CastKind::Ext : CastKind::Trunc;
			default:
				return CastKind::Invalid;
		}
	}

	std::string PointerType::ToString() const
	{
		if (BaseType.GetQuals() == 0)
			return BaseType->ToString() + "*";
		return "(" + BaseType.ToString() + ")*";
	}

	CastKind PointerType::CastTo(DataType *To) const
	{
		if (this == To)
			return CastKind::Exact;

		if (auto PtrType = Cast<PointerType>(To))
		{
			if (BaseType.HasQualifier(QualType::CONST) &&
				!PtrType->BaseType.HasQualifier(QualType::CONST))
				return CastKind::Invalid;

			if (PtrType->BaseType->IsVoidType())
				return CastKind::CategoryConv;

			if (BaseType->IsVoidType())
				return CastKind::Explicit;

			return CastKind::Invalid;
		}

		switch (To->GetCategory())
		{
			case TypeCategory::Boolean:
				return CastKind::CategoryConv;
			default:
				return CastKind::Invalid;
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

	std::string ArrayType::ToString() const
	{
		if (BaseType.GetQuals() == 0)
			return BaseType->ToString() + "[" + std::to_string(Length) + "]";
		return "(" + BaseType.ToString() + ")[" + std::to_string(Length) + "]";
	}

	CastKind ArrayType::CastTo(DataType *To) const
	{
		if (this == To) return CastKind::Exact;
		if (auto ArrType = Cast<ArrayType>(To))
			if (ArrType->BaseType.GetType() == BaseType.GetType() &&
				ArrType->Length == Length)
				return CastKind::Exact;

		if (auto PtrType = Cast<PointerType>(To))
			if (PtrType->GetBaseType() == BaseType)
				return CastKind::Explicit;

		return CastKind::Invalid;
	}

	std::string FunctionType::ToString() const
	{
		std::string Result = ReturnType.ToString() + "(";
		for (size_t i = 0; i < Params.size(); i++)
		{
			if (i == Params.size() - 1)
				Result += Params[i].ToString();
			else
				Result += Params[i].ToString() + ", ";
		}
		return Result + ")";
	}

	std::string FunctionType::GetIRName() const
	{
		std::string Result = "F" + ReturnType.GetIRName();
		for (QualType Param : Params)
			Result += Param.GetIRName();
		return Result + "E";
	}
}
