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

class LLVMCompiler
{
private:
    llvm::LLVMContext Context;
    std::unique_ptr<llvm::Module> Module = nullptr;
    llvm::IRBuilder<> Builder;

    ASTNode* ASTTree;

    std::unordered_map<std::string, llvm::Function*> Functions;
    std::unordered_map<std::string, llvm::AllocaInst*> SymbolTable;

    std::vector<std::vector<ScopeEntry>> ScopeStack;
    std::stack<llvm::BasicBlock*> LoopEndStack;
    std::stack<llvm::BasicBlock*> LoopHeaderStack;

public:
    LLVMCompiler(ASTNode* ASTTree)
        : Module(std::make_unique<llvm::Module>("volt", Context)), Builder(Context), ASTTree(ASTTree) {}

    void Compile();
    void Print() const { Module->print(llvm::outs(), nullptr); }

    int Run();

private:
    llvm::Value* CompileNode(ASTNode* Node);

    llvm::Value* CompileBlock(const BlockNode* Block);

    llvm::Value* CompileInt(const IntegerNode* Int);
    llvm::Value* CompileFloat(const FloatingPointNode* Float);
    llvm::Value* CompileBool(const BoolNode* Bool);
    llvm::Value* CompileIdentifier(const IdentifierNode* Identifier);
    llvm::Value* CompilePrefix(const PrefixOpNode* Prefix);
    llvm::Value* CompileSufix(const SuffixOpNode* Suffix);
    llvm::Value* CompileUnary(const UnaryOpNode* Unary);
    llvm::Value* CompileComparison(const ComparisonNode* Comparison);
    llvm::Value* CompileAssignment(const AssignmentNode* Assignment);
    llvm::Value* CompileBinary(const BinaryOpNode* BinaryOp);
    llvm::Value* CompileCall(const CallNode* Call);
    llvm::Value* CompileVariable(const VariableNode* Var);
    llvm::Value* CompileFunction(const FunctionNode *Function);
    llvm::Value* CompileReturn(const ReturnNode* Return);
    llvm::Value* CompileIf(const IfNode *If);
    llvm::Value* CompileWhile(const WhileNode* While);
    llvm::Value* CompileFor(const ForNode* For);
    llvm::Value* CompileBreak();
    llvm::Value* CompileContinue();

    llvm::Type* ToLLVMType(DataType Type);
    llvm::Value* CastInteger(llvm::Value* Value, llvm::Type* Target, bool Signed = true);
    void CastToJointType(llvm::Value*& Left, llvm::Value*& Right, bool Signed = true);
    llvm::Value* CastToBool(llvm::Value* Value);

    static int GetTypeRank(llvm::Type* Type);
    llvm::Value* ImplicitCast(llvm::Value* Value, llvm::Type* Target, bool Signed = true);

    void DeclareVariable(const std::string& Name, llvm::AllocaInst* AllocaInst);
    llvm::AllocaInst* GetVariable(const std::string& Name);

    void EnterScope();
    void ExitScope();

    llvm::AllocaInst* GetLValue(ASTNode* Node);
};


#endif //CVOLT_LLVMCOMPILER_H