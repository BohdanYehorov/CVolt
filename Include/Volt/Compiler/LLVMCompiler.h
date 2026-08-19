//
// Created by bohdan on 03.01.26.
//

#ifndef CVOLT_LLVMCOMPILER_H
#define CVOLT_LLVMCOMPILER_H

#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Compiler/Value/IRValue.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/TypeChecker/TypeChecker.h"
#include "IRBuilder.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <stack>

namespace Volt
{
    class LLVMCompiler
    {
        struct ScopeEntry
        {
            llvm::StringRef Name;
            IRValue* Previous = nullptr;

            ScopeEntry(llvm::StringRef Name, IRValue* Previous)
                : Name(Name), Previous(Previous) {}
        };

        struct FunctionBodyData
        {
            llvm::StringRef Name;
            BlockNode* BodyNode;
            llvm::Function* Func;
            DataType* ReturnType;
            DataType* ThisType;
            llvm::ArrayRef<ParamNode*> Params;

            FunctionBodyData(llvm::StringRef Name, BlockNode* BodyNode,
                llvm::Function* Func, DataType* ReturnType, DataType* ThisType, llvm::ArrayRef<ParamNode*> Params)
                : Name(Name), BodyNode(BodyNode),
                Func(Func), ReturnType(ReturnType), ThisType(ThisType), Params(Params) {}
        };

        using VariableTable = llvm::StringMap<IRValue*>;
        using GlobalVariableTable = llvm::StringMap<IRValue*>;

    private:
        CompilationContext& CContext;

        llvm::LLVMContext& Context;
        std::unique_ptr<llvm::Module>& Module;
        IRBuilder Builder;
        Arena& CompilerArena;

        ASTNode* ASTTree;

        GlobalVariableTable GlobalVariables;

        BuiltinFunctionTable& BuiltinFuncTable;
        VariableTable SymbolTable;

        Array<Array<ScopeEntry>> ScopeStack;
        std::stack<llvm::BasicBlock*> LoopEndStack;
        std::stack<llvm::BasicBlock*> LoopHeaderStack;

        Array<FunctionBodyData> FunctionBlocks;

        DataType* FunctionReturnType = nullptr;
        bool InFunction = false;

    public:
        LLVMCompiler(CompilationContext& CContext, BuiltinFunctionTable& BuiltinFuncTable)
            : CContext(CContext), Context(CContext.Context),
            Module(CContext.Module), Builder(CContext),
            CompilerArena(CContext.MainArena), ASTTree(CContext.ASTTree),
            BuiltinFuncTable(BuiltinFuncTable)
        {
            BuiltinFuncTable.CreateLLVMFunctions(Module.get(), Context);
        }

        void Compile();

    private:
        IRValue* GetCompileTimeValue(const ASTNode* Node);

        IRValue *CompileNode(const ASTNode *Node);
        IRValue *CompileBlock(const BlockNode *Block);
        IRValue *CompileInt(const IntegerNode *Int);
        IRValue *CompileFloat(const FloatingPointNode *Float);
        IRValue *CompileBool(const BoolNode *Bool);
        IRValue *CompileChar(const CharNode *Char);
        IRValue *CompileString(const StringNode *String);
        IRValue *CompileArray(const ArrayNode *Array);
        IRValue *CompileNullPointer(const NullPointerNode* NullPtr);
        IRValue *CompileIdentifier(const IdentifierNode *Identifier);
        IRValue *CompileRef(const RefNode *Ref);
        IRValue *CompileUnref(const UnrefNode *Unref);
        IRValue *CompilePrefix(const PrefixOpNode *Prefix);
        IRValue *CompileSuffix(const SuffixOpNode *Suffix);
        IRValue *CompileUnary(const UnaryOpNode *Unary);
        IRValue *CompileComparison(const ComparisonNode *Comparison);
        IRValue *CompileLogical(const LogicalNode *Logical);
        IRValue *CompileAssignment(const AssignmentNode *Assignment);
        IRValue *CompileBinary(const BinaryOpNode *BinaryOp);
        IRValue *CompileCall(const CallNode *Call);
        IRValue *CompileMemberAccess(const MemberAccessNode *MemberAccess);
        IRValue *CompileSubscript(const SubscriptNode *Subscript);
        IRValue *CompileExplicitCast(const ExplicitCastNode *ExplicitCast);
        IRValue *CompileConstruct(const ConstructNode *Construct);
        IRValue *CompileVariable(const VariableNode *Var);
        IRValue *CompileVariableConstruct(const VariableConstructNode *Construct);
        IRValue *CompileFunction(const FunctionNode *Function);
        IRValue *CompileReturn(const ReturnNode *Return);
        IRValue *CompileMethod(const FunctionNode *Method, ClassType *Type);
        IRValue *CompileConstructor(const ConstructorNode *Constructor, ClassType *Type);
        IRValue *CompileClass(const ClassNode *Class);
        IRValue *CompileIf(const IfNode *If);
        IRValue *CompileWhile(const WhileNode *While);
        IRValue *CompileFor(const ForNode *For);
        IRValue *CompileBreak();
        IRValue *CompileContinue();

        bool GetClassFromMemberAccess(const MemberAccessNode* MemberAccess, llvm::Value*& Value, ClassType*& Type);

        void CreateFunction(llvm::StringRef Name, llvm::ArrayRef<ParamNode*> Params,
            DataType* ReturnType, BlockNode* Body, CalleeBase* Callee,
            ArgsVector<llvm::Type*>& LLVMParams, DataType* ThisType = nullptr);

        IRValue *CompileToRValue(const ASTNode* Node)
        {
            IRValue* Value = CompileNode(Node);
            if (!Value) return nullptr;
            return Builder.CreateLoadIfLValue(Value);
        }

        IRValue* Assign(IRValue* Var, ASTNode* Value);
        IRValue* CallMethod(MemberAccessNode* Target, MethodCallee* Callee, llvm::ArrayRef<ASTNode*> ArgNodes);
        IRValue* CallConstructor(IdentifierNode* Target, FunctionCallee* Callee, llvm::ArrayRef<ASTNode*> ArgNodes);

        llvm::AllocaInst* CreateRetValueForAggregateType(DataType* RetType, ArgsVector<llvm::Value*>& Args);

        void CompileFunctionBodies();

        void DeclareVariable(llvm::StringRef Name, IRValue *Var);

        void EnterScope();
        void ExitScope();

        void FillArray(const ArrayNode *Array, llvm::AllocaInst *Alloca);
        void FillArgs(llvm::ArrayRef<ASTNode*> ParamNodes, ArgsVector<llvm::Value*>& Params);

        template <typename T, typename ...Args_>
        [[nodiscard]] T *Create(Args_&&... Args) { return CompilerArena.Create<T>(std::forward<Args_>(Args)...); }
    };
}

#endif //CVOLT_LLVMCOMPILER_H
