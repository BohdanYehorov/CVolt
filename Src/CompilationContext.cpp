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

	PointerType *CompilationContext::GetPointerType(DataType *BaseType)
	{
		PointerType PtrDataType(BaseType);

		if (auto Iter = CachedTypes.find(&PtrDataType); Iter != CachedTypes.end())
			return Cast<PointerType>(Iter->Type);

		auto PtrTypeNode = MainArena.Create<PointerType>(BaseType);
		CachedTypes.insert(PtrTypeNode);

		return PtrTypeNode;
	}

	ArrayType *CompilationContext::GetArrayType(DataType *BaseType, size_t Length)
	{
		ArrayType ArrDataType(BaseType, Length);

		if (auto Iter = CachedTypes.find(&ArrDataType); Iter != CachedTypes.end())
			return Cast<ArrayType>(Iter->Type);

		auto ArrType = MainArena.Create<ArrayType>(BaseType, Length);
		CachedTypes.insert(ArrType);

		return ArrType;
	}

	llvm::Type *CompilationContext::GetLLVMType(DataType *Type)
	{
		if (!Type->CachedType)
			Type->CachedType = DataTypeUtils::GetLLVMType(Type, Context);

		return Type->CachedType;
	}
}
