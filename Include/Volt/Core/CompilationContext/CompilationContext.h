//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_COMPILATIONCONTEXT_H
#define CVOLT_COMPILATIONCONTEXT_H

#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Types/DataTypeUtils.h"
#include "Volt/Compiler/Hash/DataTypeHash.h"
#include "Volt/Core/Lexer/Token.h"
#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/ADT/String.h"
#include <llvm/IR/LLVMContext.h>
#include <unordered_set>

namespace Volt
{
	class BuiltinFunctionTable;

	class CompilationContext
	{
	private:
		struct DataTypeWrap
		{
			DataType* Type;

			DataTypeWrap(DataType* Type) : Type(Type) {}
			operator DataType*() const { return Type; }

			bool operator==(const DataTypeWrap& Other) const
			{
				return DataTypeUtils::IsEqual(Type, Other.Type);
			}
		};

	private:
		std::unordered_set<DataTypeWrap, DataTypeHash> CachedTypes;
		VoidType* CachedVoidType = nullptr;
		BoolType* CachedBoolType = nullptr;
		CharType* CachedCharType = nullptr;
		IntegerType* CachedIntegerTypes[4]  = { nullptr, nullptr, nullptr, nullptr };
		FloatingPointType* CachedFPTypes[4] = { nullptr, nullptr, nullptr, nullptr };

	private:
		String Code;
		Arena MainArena;
		llvm::LLVMContext Context;

		// std::vector<Token> Tokens;
		Array<Token> Tokens;
		ASTNode* ASTTree = nullptr;

	public:
		CompilationContext(const String& Code)
			: Code(Code) {}

		CompilationContext(const CompilationContext&) = delete;
		CompilationContext(CompilationContext&&) = delete;
		CompilationContext& operator=(const CompilationContext&) = delete;
		CompilationContext& operator=(CompilationContext&&) = delete;

		[[nodiscard]] llvm::StringRef GetTokenLexeme(StringRef Ref) const;

		[[nodiscard]] const Array<Token>& GetTokens() const { return Tokens; }
		[[nodiscard]] const ASTNode* GetASTTree() const { return ASTTree; }

		[[nodiscard]] VoidType* GetVoidType();
		[[nodiscard]] BoolType* GetBoolType();
		[[nodiscard]] CharType* GetCharType();
		[[nodiscard]] IntegerType* GetIntegerType(size_t BitWidth);
		[[nodiscard]] FloatingPointType* GetFPType(size_t BitWidth);
		[[nodiscard]] PointerType* GetPointerType(DataType *BaseType);
		[[nodiscard]] ArrayType* GetArrayType(DataType *BaseType, size_t Length);
		[[nodiscard]] llvm::Type* GetLLVMType(DataType* Type);

		friend struct Token;
		friend class Lexer;
		friend class Parser;
		friend class TypeChecker;
		friend class LLVMCompiler;
		friend BuiltinFunctionTable;
	};
}

#endif //CVOLT_COMPILATIONCONTEXT_H