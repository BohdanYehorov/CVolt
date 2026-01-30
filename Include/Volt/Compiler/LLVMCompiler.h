//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_LLVMCOMPILER_H
#define CVOLT_LLVMCOMPILER_H

#include "Volt/Core/AST/ASTNodes.h"
#include "Types/CompilerTypes.h"
#include "Volt/Core/Memory/Arena.h"
#include "Types/TypedValue.h"
#include "Functions/FunctionSignature.h"
#include "Hash/FunctionSignatureHash.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/TypeChecker/TypeChecker.h"
#include "Volt/Core/Types/TypeDefs.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <unordered_map>
#include <stack>

namespace Volt
{
    class LLVMCompiler
    {
    private:
        llvm::LLVMContext Context;
        LLVMContextScope ContextScope{ Context };
        std::unique_ptr<llvm::Module> Module = nullptr;
        llvm::IRBuilder<> Builder;
        Arena& CompilerArena;

        ASTNode* ASTTree;
        BuiltinFunctionTable& BuiltinFuncTable;

        std::unordered_map<std::string, llvm::Function*> Functions;
        FunctionTable FunctionSignatures;
        VariableTable SymbolTable;
        std::unordered_map<std::string, llvm::orc::ExecutorSymbolDef> DefaultSymbols;
        std::unordered_map<FunctionSignature,
        std::pair<std::string, DataType>, FunctionSignatureHash> DefaultFunctionSignatures;

        std::vector<std::vector<ScopeEntry>> ScopeStack;
        std::stack<llvm::BasicBlock*> LoopEndStack;
        std::stack<llvm::BasicBlock*> LoopHeaderStack;

        llvm::Function* CurrentFunction = nullptr;
        llvm::ArrayRef<DataType> FunctionParams;

    public:
        LLVMCompiler(Arena& CompilerArena, TypeChecker& TyChecker)
            : Module(std::make_unique<llvm::Module>("volt", Context)), Builder(Context),
            CompilerArena(CompilerArena), ASTTree(TyChecker.GetASTTree()),
            BuiltinFuncTable(TyChecker.GetBuiltinFunctionTable()),
            FunctionSignatures(TyChecker.GetFunctions())
        {
            BuiltinFuncTable.CreateLLVMFunctions(Module.get());
        }

        void Compile();
        void Write(llvm::raw_ostream& Os) const { Module->print(Os, nullptr); }
        void Print() const { Module->print(llvm::outs(), nullptr); }

        int Run();

    private:
        TypedValue *CompileNode(ASTNode *Node);
        TypedValue *CompileBlock(const BlockNode *Block);
        TypedValue *CompileInt(const IntegerNode *Int);
        TypedValue *CompileFloat(const FloatingPointNode *Float);
        TypedValue *CompileBool(const BoolNode *Bool);
        TypedValue *CompileChar(const CharNode *Char);
        TypedValue *CompileString(const StringNode *String);
        TypedValue *CompileArray(const ArrayNode *Array);
        TypedValue *CompileIdentifier(const IdentifierNode *Identifier);
        TypedValue *CompileRef(const RefNode *Ref);
        TypedValue *CompilePrefix(const PrefixOpNode *Prefix);
        TypedValue *CompileSuffix(const SuffixOpNode *Suffix);
        TypedValue *CompileUnary(const UnaryOpNode *Unary);
        TypedValue *CompileComparison(const ComparisonNode *Comparison);
        TypedValue *CompileLogical(const LogicalNode *Logical);
        TypedValue *CompileAssignment(const AssignmentNode *Assignment);
        TypedValue *CompileBinary(const BinaryOpNode *BinaryOp);
        TypedValue *CompileCall(const CallNode *Call);
        TypedValue *CompileSubscript(const SubscriptNode *Subscript);
        TypedValue *CompileVariable(const VariableNode *Var);
        TypedValue *CompileFunction(const FunctionNode *Function);
        TypedValue *CompileReturn(const ReturnNode *Return);
        TypedValue *CompileIf(const IfNode *If);
        TypedValue *CompileWhile(const WhileNode *While);
        TypedValue *CompileFor(const ForNode *For);
        TypedValue *CompileBreak();
        TypedValue *CompileContinue();

        void DeclareVariable(const std::string& Name, TypedValue *Var);
        TypedValue *GetVariable(const std::string &Name);

        void EnterScope();
        void ExitScope();

        TypedValue *GetLValue(const ASTNode *Node);

        TypedValue *ImplicitCast(TypedValue *Value, DataType Target);
        static bool CanImplicitCast(DataType Src, DataType Dst);

        static bool GetIntegerValue(const ASTNode *Node, Int64 &Num);

        void FillArray(const ArrayNode *Array, llvm::AllocaInst *Alloca);

        template <typename T, typename ...Args_>
        [[nodiscard]] T *Create(Args_&&... Args) { return CompilerArena.Create<T>(std::forward<Args_>(Args)...); }
    };
}

#endif //CVOLT_LLVMCOMPILER_H
