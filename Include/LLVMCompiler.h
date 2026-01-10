//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_LLVMCOMPILER_H
#define CVOLT_LLVMCOMPILER_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include "ASTNodes.h"
#include <unordered_map>
#include <stack>
#include "Hash.h"
#include "CompilerTypes.h"
#include "Arena.h"
#include "TypedValue.h"
#include "FunctionSignature.h"

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
        std::unordered_map<FunctionSignature, TypedFunction*, FunctionSignatureHash> FunctionSignatures;
        std::unordered_map<std::string, TypedValue*> SymbolTable;
        std::unordered_map<std::string, llvm::orc::ExecutorSymbolDef> DefaultSymbols;

        std::vector<std::vector<ScopeEntry>> ScopeStack;
        std::stack<llvm::BasicBlock*> LoopEndStack;
        std::stack<llvm::BasicBlock*> LoopHeaderStack;

        llvm::Function* CurrentFunction = nullptr;
        llvm::ArrayRef<DataTypeNodeBase*> FunctionParams;

    public:
        LLVMCompiler(ASTNode* ASTTree)
            : Module(std::make_unique<llvm::Module>("volt", Context)), Builder(Context), ASTTree(ASTTree) {}

        void Compile();
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

        template <typename T>
        void CreateDefaultFunction(const std::string& Name, T* FuncPtr, llvm::Type* RetType,
            const llvm::SmallVector<llvm::Type*, 8>& Params);

        void CastToJointType(TypedValue *&Left, TypedValue *&Right);
        TypedValue *ImplicitCast(TypedValue *Value, DataType *Target);

        template <typename T, typename ...Rest>
        void FillParams(llvm::SmallVector<llvm::Type*, 8>& Params);

        template <typename Ret, typename ...Args>
        void CreateDefaultFunction(const std::string& Name, Ret(*FuncPtr)(Args...));

        template <typename Ret>
        void CreateDefaultFunction(const std::string& Name, Ret(*FuncPtr)());

        template <typename T, typename ...Args_>
        [[nodiscard]] T *Create(Args_&&... Args) { return CompilerArena.Create<T>(std::forward<Args_>(Args)...); }
    };

    template<typename T>
    void LLVMCompiler::CreateDefaultFunction(const std::string &Name, T *FuncPtr, llvm::Type *RetType,
        const llvm::SmallVector<llvm::Type *, 8> &Params)
    {
        llvm::FunctionType* FuncType = llvm::FunctionType::get(RetType, Params, false);
        llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, Name, Module.get());

        DefaultSymbols[Name] = llvm::orc::ExecutorSymbolDef(
            llvm::orc::ExecutorAddr::fromPtr(FuncPtr),
            llvm::JITSymbolFlags::Exported);
    }

    template <typename>
    llvm::Type* GetLLVMType(llvm::LLVMContext& Context);
    template<> inline llvm::Type* GetLLVMType<void>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getVoidTy(Context);
    }

    template<> inline llvm::Type* GetLLVMType<bool>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getInt1Ty(Context);
    }

    template<> inline llvm::Type* GetLLVMType<char>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getInt8Ty(Context);
    }

    template<> inline llvm::Type* GetLLVMType<std::byte>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getInt8Ty(Context);
    }

    template<> inline llvm::Type* GetLLVMType<int>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getInt32Ty(Context);
    }

    template<> inline llvm::Type* GetLLVMType<long>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getInt64Ty(Context);
    }

    template<> inline llvm::Type* GetLLVMType<float>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getFloatTy(Context);
    }

    template<> inline llvm::Type* GetLLVMType<double>(llvm::LLVMContext& Context)
    {
        return llvm::Type::getDoubleTy(Context);
    }

    template<> inline llvm::Type* GetLLVMType<void*>(llvm::LLVMContext& Context)
    {
        return llvm::PointerType::get(Context, 0);
    }

    template<> inline llvm::Type* GetLLVMType<const char*>(llvm::LLVMContext& Context)
    {
        return llvm::PointerType::get(Context, 0);
    }

    template<typename T, typename ... Rest>
    void LLVMCompiler::FillParams(llvm::SmallVector<llvm::Type *, 8> &Params)
    {
        Params.push_back(GetLLVMType<T>(Context));
        if constexpr (sizeof...(Rest) > 0)
            FillParams<Rest...>(Params);
    }

    template<typename Ret, typename ... Args>
    void LLVMCompiler::CreateDefaultFunction(const std::string &Name, Ret(*FuncPtr)(Args...))
    {
        llvm::SmallVector<llvm::Type*, 8> Params;
        FillParams<Args...>(Params);

        CreateDefaultFunction(Name, FuncPtr, GetLLVMType<Ret>(Context), Params);
    }

    template<typename Ret>
    void LLVMCompiler::CreateDefaultFunction(const std::string &Name, Ret(*FuncPtr)())
    {
        CreateDefaultFunction(Name, FuncPtr, GetLLVMType<Ret>(Context), {});
    }
}

#endif //CVOLT_LLVMCOMPILER_H
