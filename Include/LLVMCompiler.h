//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_LLVMCOMPILER_H
#define CVOLT_LLVMCOMPILER_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include "ASTNodes.h"
#include <unordered_map>
#include <stack>
#include "Hash.h"
#include "CompilerTypes.h"
#include "Arena.h"
#include "TypedValue.h"

namespace Volt
{
    class LLVMCompiler
    {
    private:
        llvm::LLVMContext Context;
        std::unique_ptr<llvm::Module> Module = nullptr;
        llvm::IRBuilder<> Builder;
        Arena CompilerArena;

        ASTNode* ASTTree;

        std::unordered_map<std::string, llvm::Function*> Functions;
        std::unordered_map<std::string, llvm::AllocaInst*> SymbolTable;

        std::vector<std::vector<ScopeEntry>> ScopeStack;
        std::stack<llvm::BasicBlock*> LoopEndStack;
        std::stack<llvm::BasicBlock*> LoopHeaderStack;

        llvm::Function* CurrentFunction = nullptr;

    public:
        LLVMCompiler(ASTNode* ASTTree)
            : Module(std::make_unique<llvm::Module>("volt", Context)), Builder(Context), ASTTree(ASTTree) {}

        void Compile();
        void Print() const { Module->print(llvm::outs(), nullptr); }

        int Run();

    private:
        TypedValue CompileNode(ASTNode *Node);

        TypedValue CompileBlock(const BlockNode *Block);

        TypedValue CompileInt(const IntegerNode *Int);
        TypedValue CompileFloat(const FloatingPointNode *Float);
        TypedValue CompileBool(const BoolNode *Bool);
        TypedValue CompileChar(const CharNode *Char);
        TypedValue CompileString(const StringNode *String);
        TypedValue CompileIdentifier(const IdentifierNode *Identifier);
        TypedValue CompileRef(const RefNode *Ref);
        TypedValue CompilePrefix(const PrefixOpNode *Prefix);
        TypedValue CompileSuffix(const SuffixOpNode *Suffix);
        TypedValue CompileUnary(const UnaryOpNode *Unary);
        TypedValue CompileComparison(const ComparisonNode *Comparison);
        TypedValue CompileAssignment(const AssignmentNode *Assignment);
        TypedValue CompileBinary(const BinaryOpNode *BinaryOp);
        TypedValue CompileCall(const CallNode *Call);
        TypedValue CompileVariable(const VariableNode *Var);
        TypedValue CompileFunction(const FunctionNode *Function);
        TypedValue CompileReturn(const ReturnNode *Return);
        TypedValue CompileIf(const IfNode *If);
        TypedValue CompileWhile(const WhileNode *While);
        TypedValue CompileFor(const ForNode *For);
        TypedValue CompileBreak();
        TypedValue CompileContinue();

        llvm::Type* ToLLVMType(PrimitiveDataType Type);
        llvm::Value* CastInteger(llvm::Value* Value, llvm::Type* Target, bool Signed = true);
        void CastToJointType(llvm::Value*& Left, llvm::Value*& Right, bool Signed = true);
        llvm::Value* CastToBool(llvm::Value* Value);

        static int GetTypeRank(llvm::Type* Type);
        llvm::Value* ImplicitCast(llvm::Value* Value, llvm::Type* Target, bool Signed = true);

        void DeclareVariable(const std::string& Name, llvm::AllocaInst* AllocaInst);
        llvm::AllocaInst* GetVariable(const std::string& Name);

        void EnterScope();
        void ExitScope();

        llvm::Type* CompileType(const DataTypeNodeBase* Type);
        llvm::AllocaInst* GetLValue(const ASTNode* Node);

        void CreateDefaultFunction(const std::string& Name, llvm::Type* RetType,
            const llvm::SmallVector<llvm::Type*, 8>& Params);

        TypedValue ImplicitCast(const TypedValue& Value, DataTypeNodeBase* Target);
    };
}

#endif //CVOLT_LLVMCOMPILER_H
