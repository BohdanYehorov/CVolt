//
// Created by bohdan on 06.02.26.
//

#ifndef CVOLT_COMPILATIONCONTEXT_H
#define CVOLT_COMPILATIONCONTEXT_H

#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Types/ClassType.h"
#include "Volt/Core/Lexer/Token.h"
#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/ADT/String.h"
#include "Volt/Core/Errors/LexError.h"
#include "Volt/Core/Errors/ParseError.h"
#include "Volt/Core/Errors/TypeError.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace Volt
{
	class CompilationContext
	{
	private:
		VoidType* CachedVoidType = nullptr;
		BoolType* CachedBoolType = nullptr;
		CharType* CachedCharType = nullptr;
		IntegerType* CachedIntegerTypes[8]  = { nullptr, nullptr, nullptr, nullptr,
												nullptr, nullptr, nullptr, nullptr };
		FloatingPointType* CachedFPTypes[4] = { nullptr, nullptr, nullptr, nullptr };
		NullPointerType* CachedNullPtrType = nullptr;

		llvm::FoldingSet<PointerType> PointerTypes;
		llvm::FoldingSet<ReferenceType> ReferenceTypes;
		llvm::FoldingSet<ArrayType> ArrayTypes;
		llvm::StringMap<ClassType*> ClassTypes;

	private:
		String Code;
		Arena MainArena;
		llvm::LLVMContext Context;
		std::unique_ptr<llvm::Module> Module = nullptr;

		Array<Token> Tokens;
		ASTNode* ASTTree = nullptr;

		Array<LexError> LexErrors;
		Array<ParseError> ParseErrors;
		Array<TypeError> TypeErrors;

	public:
		CompilationContext(String&& Code, llvm::StringRef FileName)
			: Code(std::move(Code)), Module(std::make_unique<llvm::Module>(FileName, Context)) {}

		CompilationContext(const CompilationContext&) = delete;
		CompilationContext(CompilationContext&&) = delete;
		CompilationContext& operator=(const CompilationContext&) = delete;
		CompilationContext& operator=(CompilationContext&&) = delete;

		[[nodiscard]] llvm::StringRef GetTokenLexeme(StringRef Ref) const;

		[[nodiscard]] const String& GetSourceCode() const { return Code; }
		[[nodiscard]] const Array<Token>& GetTokens() const { return Tokens; }
		[[nodiscard]] const ASTNode* GetASTTree() const { return ASTTree; }

		[[nodiscard]] VoidType* GetVoidType();
		[[nodiscard]] BoolType* GetBoolType();
		[[nodiscard]] CharType* GetCharType();
		[[nodiscard]] IntegerType* GetIntegerType(size_t BitWidth, bool IsSigned = true);
		[[nodiscard]] FloatingPointType* GetFPType(size_t BitWidth);
		[[nodiscard]] PointerType* GetPointerType(QualType BaseType);
		[[nodiscard]] NullPointerType* GetNullPointerType();
		[[nodiscard]] ReferenceType* GetReferenceType(QualType BaseType);
		[[nodiscard]] ArrayType* GetArrayType(QualType BaseType, size_t Length);

		ClassType* CreateClassType(llvm::StringRef Name, const Array<Field>& Fields);
		ClassType* CreateClassType(llvm::StringRef Name);
		[[nodiscard]] ClassType* GetClassType(llvm::StringRef Name);

		ClassType* GetOrCreateClassType(llvm::StringRef Name, const Array<Field> &Fields);

		[[nodiscard]] llvm::Type* GetLLVMType(DataType* Type);

		[[nodiscard]] bool HasErrors() const { return !LexErrors.Empty() ||
			!ParseErrors.Empty() || !TypeErrors.Empty(); }

		friend struct Token;
		friend class Lexer;
		friend class Parser;
		friend class TypeChecker;
		friend class LLVMCompiler;
		friend class BuiltinFunctionTable;
		friend class JITEngine;
		friend class ExprResult;
		friend class ExprAddress;
		friend class IRValue;
		friend class DebugOutput;
		friend class ClassInst;
		friend class IRBuilder;
	};
}

#endif //CVOLT_COMPILATIONCONTEXT_H