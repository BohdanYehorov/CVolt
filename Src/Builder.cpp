//
// Created by bohdan on 03.02.26.
//

#include "Volt/Core/Builder/Builder.h"
#include <cmath>

namespace Volt
{
	VoidType *BuilderBase::GetVoidType() const
	{
		if (!CachedVoidType)
			CachedVoidType = MainArena.Create<VoidType>();

		return CachedVoidType;
	}

	BoolType *BuilderBase::GetBoolType() const
	{
		if (!CachedBoolType)
			CachedBoolType = MainArena.Create<BoolType>();

		return CachedBoolType;
	}

	CharType *BuilderBase::GetCharType() const
	{
		if (!CachedCharType)
			CachedCharType = MainArena.Create<CharType>();

		return CachedCharType;
	}

	IntegerType *BuilderBase::GetIntegerType(size_t BitWidth) const
	{
		static size_t MinBitWidth = 8;

		assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 64);

		auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
		if (Index >= std::size(CachedIntegerTypes))
			throw std::range_error("Out of range");

		if (!CachedIntegerTypes[Index])
			CachedIntegerTypes[Index] = MainArena.Create<IntegerType>(BitWidth);

		return CachedIntegerTypes[Index];
	}

	FloatingPointType *BuilderBase::GetFPType(size_t BitWidth) const
	{
		static size_t MinBitWidth = 16;
		assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 128);

		auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
		if (Index >= std::size(CachedFPTypes))
			throw std::range_error("Out of range");

		if (!CachedFPTypes[Index])
			CachedFPTypes[Index] = MainArena.Create<FloatingPointType>(BitWidth);

		return CachedFPTypes[Index];
	}

	PointerType *BuilderBase::GetPointerType(DataTypeBase *BaseType) const
	{
		PointerType PtrDataType(BaseType);

		if (auto Iter = CachedTypes.find(&PtrDataType); Iter != CachedTypes.end())
			return Cast<PointerType>(Iter->Type);

		auto PtrTypeNode = MainArena.Create<PointerType>(BaseType);
		CachedTypes.insert(PtrTypeNode);

		return PtrTypeNode;
	}

	ArrayType *BuilderBase::GetArrayType(DataTypeBase *BaseType, size_t Length) const
	{
		ArrayType ArrDataType(BaseType, Length);

		if (auto Iter = CachedTypes.find(&ArrDataType); Iter != CachedTypes.end())
			return Cast<ArrayType>(Iter->Type);

		auto ArrType = MainArena.Create<ArrayType>(BaseType, Length);
		CachedTypes.insert(ArrType);

		return ArrType;
	}

	int BuilderBase::GetTypeRank(DataTypeBase *Type)
	{
		if (const auto PrimitiveType =  Cast<const PrimitiveDataType>(Type))
			return DataType::GetPrimitiveTypeRank(PrimitiveType);

		static int MaxPrimitiveTypeRank = DataType::GetPrimitiveTypeRank(
			Cast<PrimitiveDataType>(GetFPType(128)));

		if (Cast<const PointerType>(Type))
			return MaxPrimitiveTypeRank + 1;

		if (Cast<const ArrayType>(Type))
			return MaxPrimitiveTypeRank + 2;

		return -1;
	}

	LLVMBuilder::LLVMBuilder(BuilderBase &Other, llvm::LLVMContext &Context)
		: BuilderBase(Other.MainArena), Context(Context)
	{
		if (&MainArena == &Other.MainArena)
		{
			CachedTypes = Other.CachedTypes;

			CachedVoidType = Other.CachedVoidType;
			CachedBoolType = Other.CachedBoolType;
			CachedCharType = Other.CachedCharType;

			for (size_t i = 0; i < std::size(CachedIntegerTypes); i++)
				CachedIntegerTypes[i] = Other.CachedIntegerTypes[i];

			for (size_t i = 0; i < std::size(CachedFPTypes); i++)
				CachedFPTypes[i] = Other.CachedFPTypes[i];
		}
	}
}
