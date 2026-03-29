//
// Created by bohdan on 06.02.26.
//

#include "Volt/Core/CompilationContext/CompilationContext.h"
#include <cmath>

namespace Volt
{
	llvm::StringRef CompilationContext::GetTokenLexeme(StringRef Ref) const
	{
		if (Ref.Ptr + Ref.Length > Code.Length())
			throw std::runtime_error("Ref out of code length");

		return { Code.CStr() + Ref.Ptr, Ref.Length };
	}

	VoidType *CompilationContext::GetVoidType()
	{
		if (!CachedVoidType)
			CachedVoidType = MainArena.Create<VoidType>();

		return CachedVoidType;
	}

	BoolType *CompilationContext::GetBoolType()
	{
		if (!CachedBoolType)
			CachedBoolType = MainArena.Create<BoolType>();

		return CachedBoolType;
	}

	CharType *CompilationContext::GetCharType()
	{
		if (!CachedCharType)
			CachedCharType = MainArena.Create<CharType>();

		return CachedCharType;
	}

	IntegerType *CompilationContext::GetIntegerType(size_t BitWidth)
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

	FloatingPointType *CompilationContext::GetFPType(size_t BitWidth)
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

	PointerType *CompilationContext::GetPointerType(QualType BaseType)
	{
		if (!BaseType->PointerVariants)
			BaseType->InitPointerVariants();

		size_t Index = BaseType.GetQuals();

		if (!BaseType->PointerVariants[Index])
			BaseType->PointerVariants[Index] = MainArena.Create<PointerType>(BaseType);

		return BaseType->PointerVariants[Index];
	}

	ReferenceType *CompilationContext::GetReferenceType(QualType BaseType)
	{
		if (!BaseType->ReferenceVariants)
			BaseType->InitReferenceVariants();

		size_t Index = BaseType.GetQuals();

		if (!BaseType->ReferenceVariants[Index])
			BaseType->ReferenceVariants[Index] = MainArena.Create<ReferenceType>(BaseType);

		return BaseType->ReferenceVariants[Index];
	}

	ArrayType *CompilationContext::GetArrayType(QualType BaseType, size_t Length)
	{
		if (!BaseType->ArrayVariants)
			BaseType->InitArrayVariants();

		size_t Index = BaseType.GetQuals();

		if (!BaseType->ArrayVariants[Index])
			BaseType->ArrayVariants[Index] = MainArena.Create<ArrayType>(BaseType, Length);

		return BaseType->ArrayVariants[Index];
	}

	llvm::Type *CompilationContext::GetLLVMType(DataType *Type)
	{
		assert(Type);

		if (!Type->CachedType)
			Type->CachedType = Type->ToLLVMType(Context);

		return Type->CachedType;
	}
}
