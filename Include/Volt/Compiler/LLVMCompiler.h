//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_LLVMCOMPILER_H
#define CVOLT_LLVMCOMPILER_H

#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/Compiler/Hash/Hash.h"
#include "Types/CompilerTypes.h"
#include "Volt/Core/Memory/Arena.h"
#include "Types/TypedValue.h"
#include "Functions/FunctionSignature.h"
#include "Hash/FunctionSignatureHash.h"
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
        Arena CompilerArena;

        ASTNode* ASTTree;

        std::unordered_map<std::string, llvm::Function*> Functions;
        std::unordered_map<FunctionSignature, TypedFunction*, FunctionSignatureHash> FunctionSignatures;
        std::unordered_map<std::string, TypedValue*> SymbolTable;
        std::unordered_map<std::string, llvm::orc::ExecutorSymbolDef> DefaultSymbols;
        std::unordered_map<FunctionSignature,
        std::pair<std::string, DataType>, FunctionSignatureHash> DefaultFunctionSignatures;

        std::vector<std::vector<ScopeEntry>> ScopeStack;
        std::stack<llvm::BasicBlock*> LoopEndStack;
        std::stack<llvm::BasicBlock*> LoopHeaderStack;

        llvm::Function* CurrentFunction = nullptr;
        llvm::ArrayRef<DataType> FunctionParams;

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
        TypedValue *CompileArray(const ArrayNode* Array);
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

        template <typename T>
        void CreateDefaultFunction(const std::string& Name, T* FuncPtr, llvm::Type* RetType,
            const llvm::SmallVector<llvm::Type*, 8>& Params);

        void CastToJointType(TypedValue *&Left, TypedValue *&Right);
        TypedValue *ImplicitCast(TypedValue *Value, DataType Target);
        static bool CanImplicitCast(DataType Src, DataType Dst);

        template <typename T, typename ...Rest>
        void FillParams(llvm::SmallVector<DataType, 8>& Params);

        template <typename Ret, typename ...Args>
        void CreateDefaultFunction(const std::string& SignatureName, const std::string& Name, Ret(*FuncPtr)(Args...));

        template <typename Ret>
        void CreateDefaultFunction(const std::string& SignatureName, const std::string& Name, Ret(*FuncPtr)());

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
    DataType GetDataType(Arena& TypesArena);

    template<> inline DataType GetDataType<void>(Arena& TypesArena)
    {
        return DataType::CreateVoid(TypesArena);
    }

    template<> inline DataType GetDataType<bool>(Arena& TypesArena)
    {
        return DataType::CreateBoolean(TypesArena);
    }

    template<> inline DataType GetDataType<char>(Arena& TypesArena)
    {
        return DataType::CreateChar(TypesArena);
    }

    template<> inline DataType GetDataType<std::byte>(Arena& TypesArena)
    {
        return DataType::CreateInteger(8, TypesArena);
    }

    template<> inline DataType GetDataType<short>(Arena& TypesArena)
    {
        return DataType::CreateInteger(16, TypesArena);
    }

    template<> inline DataType GetDataType<int>(Arena& TypesArena)
    {
        return DataType::CreateInteger(32, TypesArena);
    }

    template<> inline DataType GetDataType<long>(Arena& TypesArena)
    {
        return DataType::CreateInteger(64, TypesArena);
    }

    template<> inline DataType GetDataType<long long>(Arena& TypesArena)
    {
        return DataType::CreateInteger(64, TypesArena);
    }

    template<> inline DataType GetDataType<float>(Arena& TypesArena)
    {
        return DataType::CreateFloatingPoint(32, TypesArena);
    }

    template<> inline DataType GetDataType<double>(Arena& TypesArena)
    {
        return DataType::CreateFloatingPoint(64, TypesArena);
    }

    template<> inline DataType GetDataType<const char*>(Arena& TypesArena)
    {
        return DataType::CreatePtr(DataType(DataType::CreateChar(TypesArena)).GetTypeBase(), TypesArena);
    }

    template<typename T, typename ... Rest>
    void LLVMCompiler::FillParams(llvm::SmallVector<DataType, 8> &Params)
    {
        Params.push_back(GetDataType<T>(CompilerArena));
        if constexpr (sizeof...(Rest) > 0)
            FillParams<Rest...>(Params);
    }

    template<typename Ret, typename ... Args>
    void LLVMCompiler::CreateDefaultFunction(const std::string& SignatureName, const std::string &Name, Ret(*FuncPtr)(Args...))
    {
        llvm::SmallVector<DataType, 8> Params;
        FillParams<Args...>(Params);

        llvm::SmallVector<llvm::Type*, 8> LLVMParams;
        llvm::SmallVector<DataType, 8> BaseParams;

        LLVMParams.reserve(Params.size());
        BaseParams.reserve(Params.size());

        for (const auto& Param : Params)
        {
            LLVMParams.push_back(Param.GetLLVMType(Context));
            BaseParams.push_back(Param);
        }

        DataType RetType = GetDataType<Ret>(CompilerArena);

        FunctionSignature Signature{ SignatureName, BaseParams};
        DefaultFunctionSignatures[Signature] = std::make_pair(Name, RetType);

        CreateDefaultFunction(Name, FuncPtr, RetType.GetLLVMType(Context), LLVMParams);
    }

    template<typename Ret>
    void LLVMCompiler::CreateDefaultFunction(const std::string& SignatureName, const std::string &Name, Ret(*FuncPtr)())
    {
        DataType RetType = GetDataType<Ret>(CompilerArena);

        FunctionSignature Signature{ SignatureName, {}};
        DefaultFunctionSignatures[Signature] = std::make_pair(Name, RetType);;

        CreateDefaultFunction(Name, FuncPtr, RetType.GetLLVMType(Context), {});
    }
}

#endif //CVOLT_LLVMCOMPILER_H
