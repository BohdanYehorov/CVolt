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

	IntegerType *CompilationContext::GetIntegerType(size_t BitWidth, bool IsSigned)
	{
		assert(BitWidth % 8 == 0 && BitWidth >= 8 && BitWidth <= 64);

		auto Index = std::countr_zero(BitWidth) - 3 + 4 * static_cast<int>(!IsSigned);

		if (Index >= std::size(CachedIntegerTypes))
			llvm_unreachable("Out of range");

		if (!CachedIntegerTypes[Index])
			CachedIntegerTypes[Index] = MainArena.Create<IntegerType>(BitWidth, IsSigned);

		return CachedIntegerTypes[Index];
	}

	FloatingPointType *CompilationContext::GetFPType(size_t BitWidth)
	{
		assert(BitWidth % 8 == 0 && BitWidth >= 16 && BitWidth <= 128);

		auto Index = std::countr_zero(BitWidth) - 4;
		if (Index >= std::size(CachedFPTypes))
			llvm_unreachable("Out of range");

		if (!CachedFPTypes[Index])
			CachedFPTypes[Index] = MainArena.Create<FloatingPointType>(BitWidth);

		return CachedFPTypes[Index];
	}

	PointerType *CompilationContext::GetPointerType(QualType BaseType)
	{
		llvm::FoldingSetNodeID ID;
		PointerType::Profile(ID, BaseType);

		void* InsertPos = nullptr;
		if (PointerType* Type = PointerTypes.FindNodeOrInsertPos(ID, InsertPos))
			return Type;

		auto Type = MainArena.Create<PointerType>(BaseType);
		PointerTypes.InsertNode(Type, InsertPos);
		return Type;
	}

	ReferenceType *CompilationContext::GetReferenceType(QualType BaseType)
	{
		llvm::FoldingSetNodeID ID;
		ReferenceType::Profile(ID, BaseType);

		void* InsertPos = nullptr;
		if (ReferenceType* Type = ReferenceTypes.FindNodeOrInsertPos(ID, InsertPos))
			return Type;

		auto Type = MainArena.Create<ReferenceType>(BaseType);
		ReferenceTypes.InsertNode(Type, InsertPos);
		return Type;
	}

	ArrayType *CompilationContext::GetArrayType(QualType BaseType, size_t Length)
	{
		llvm::FoldingSetNodeID ID;
		ArrayType::Profile(ID, BaseType, Length, true);

		void* InsertPos = nullptr;
		if (ArrayType* Type = ArrayTypes.FindNodeOrInsertPos(ID, InsertPos))
			return Type;

		auto Type = MainArena.Create<ArrayType>(BaseType, Length);
		ArrayTypes.InsertNode(Type, InsertPos);
		return Type;
	}

	llvm::Type *CompilationContext::GetLLVMType(DataType *Type)
	{
		assert(Type);

		if (!Type->CachedType)
			Type->CachedType = Type->ToLLVMType(Context);

		return Type->CachedType;
	}
}
