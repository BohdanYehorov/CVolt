//
// Created by bohdan on 06.02.26.
//

#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Support/ErrorHandling.h"

namespace Volt
{
	llvm::StringRef CompilationContext::GetTokenLexeme(StringRef Ref) const
	{
		if (Ref.Index + Ref.Length > Code.Length())
			VoltUnreachable("Ref out of code length");

		return { Code.CStr() + Ref.Index, Ref.Length };
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
		VoltAssert(BitWidth % 8 == 0 && BitWidth >= 8 && BitWidth <= 64);

		auto Index = std::countr_zero(BitWidth) - 3 + 4 * static_cast<int>(!IsSigned);

		if (Index >= std::size(CachedIntegerTypes))
			VoltUnreachable("Out of range");

		if (!CachedIntegerTypes[Index])
			CachedIntegerTypes[Index] = MainArena.Create<IntegerType>(BitWidth, IsSigned);

		return CachedIntegerTypes[Index];
	}

	FloatingPointType *CompilationContext::GetFPType(size_t BitWidth)
	{
		VoltAssert(BitWidth % 8 == 0 && BitWidth >= 16 && BitWidth <= 128);

		auto Index = std::countr_zero(BitWidth) - 4;
		if (Index >= std::size(CachedFPTypes))
			VoltUnreachable("Out of range");

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

	NullPointerType *CompilationContext::GetNullPointerType()
	{
		if (CachedNullPtrType)
			return CachedNullPtrType;

		return MainArena.Create<NullPointerType>();
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

	ClassType* CompilationContext::CreateClassType(const std::string &Name, const Array<Field> &Fields)
	{
		if (ClassTypes.contains(Name)) return nullptr;
		ClassType* Type = MainArena.Create<ClassType>(Name, Fields);
		ClassTypes[Name] = Type;
		return Type;
	}

	ClassType* CompilationContext::GetClassType(const std::string &Name)
	{
		if (auto Iter = ClassTypes.find(Name); Iter != ClassTypes.end())
			return Iter->second;
		return nullptr;
	}

	llvm::Type *CompilationContext::GetLLVMType(DataType *Type)
	{
		VoltAssert(Type);

		if (!Type->CachedType)
			Type->CachedType = Type->ToLLVMType(Context);

		return Type->CachedType;
	}
}
