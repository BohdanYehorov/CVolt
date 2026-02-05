//
// Created by bohdan on 03.02.26.
//

#ifndef CVOLT_BUILDER_H
#define CVOLT_BUILDER_H

#include "Volt/Core/Memory/Arena.h"
#include "Volt/Compiler/Types/DataTypeUtils.h"
#include <unordered_set>
#include <llvm/IR/LLVMContext.h>

namespace Volt
{
	class BuilderBase
	{
	private:
		struct DataTypeNodeWrap
		{
			DataType* Type;

			DataTypeNodeWrap(DataType* Type) : Type(Type) {}
			operator DataType*() const { return Type; }

			bool operator==(const DataTypeNodeWrap& Other) const
			{
				return DataTypeUtils::IsEqual(Type, Other.Type);
			}
		};

	private:
		mutable std::unordered_set<DataTypeNodeWrap, DataTypeHash> CachedTypes;
		mutable VoidType* CachedVoidType = nullptr;
		mutable BoolType* CachedBoolType = nullptr;
		mutable CharType* CachedCharType = nullptr;
		mutable IntegerType* CachedIntegerTypes[4]  = { nullptr, nullptr, nullptr, nullptr };
		mutable FloatingPointType* CachedFPTypes[4] = { nullptr, nullptr, nullptr, nullptr };

	protected:
		Arena& MainArena;

	public:
		BuilderBase(Arena& MainArena)
			: MainArena(MainArena) {}

		BuilderBase(const BuilderBase&) = delete;
		BuilderBase(BuilderBase&&) = delete;
		BuilderBase& operator=(const BuilderBase&) = delete;
		BuilderBase& operator=(BuilderBase&&) = delete;

		[[nodiscard]] VoidType* GetVoidType() const;
		[[nodiscard]] BoolType* GetBoolType() const;
		[[nodiscard]] CharType* GetCharType() const;
		[[nodiscard]] IntegerType* GetIntegerType(size_t BitWidth) const;
		[[nodiscard]] FloatingPointType* GetFPType(size_t BitWidth) const;
		[[nodiscard]] PointerType* GetPointerType(DataType *BaseType) const;
		[[nodiscard]] ArrayType* GetArrayType(DataType *BaseType, size_t Length) const;

		[[nodiscard]] int GetTypeRank(DataType* Type);

		friend class LLVMBuilder;
	};

	class LLVMBuilder : public BuilderBase
	{
	private:
		llvm::LLVMContext& Context;

	public:
		LLVMBuilder(Arena& MainArena, llvm::LLVMContext& Context)
			: BuilderBase(MainArena), Context(Context) {}

		LLVMBuilder(BuilderBase& Other, llvm::LLVMContext& Context);

		[[nodiscard]] llvm::Type* GetLLVMType(DataType* Type) const
		{
			return DataTypeUtils::GetLLVMType(Type, Context);
		}
	};
}

#endif //CVOLT_BUILDER_H