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
            std::string Name;
            IRValue* Previous = nullptr;

            ScopeEntry(const std::string& Name, IRValue* Previous)
                : Name(Name), Previous(Previous) {}
        };

        using VariableTable = llvm::StringMap<IRValue*>;

    private:
        CompilationContext& CContext;

        llvm::LLVMContext& Context;
        std::unique_ptr<llvm::Module>& Module;
        llvm::IRBuilder<> Builder;
        Arena& CompilerArena;

        ASTNode* ASTTree;
        BuiltinFunctionTable& BuiltinFuncTable;
        VariableTable SymbolTable;

        Array<Array<ScopeEntry>> ScopeStack;
        std::stack<llvm::BasicBlock*> LoopEndStack;
        std::stack<llvm::BasicBlock*> LoopHeaderStack;

        DataType* FunctionReturnType = nullptr;

    public:
        LLVMCompiler(CompilationContext& CContext, BuiltinFunctionTable& BuiltinFuncTable)
            : CContext(CContext), Context(CContext.Context),
            Module(CContext.Module), Builder(Context),
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
        IRValue *CompileVariable(const VariableNode *Var);
        IRValue *CompileFunction(const FunctionNode *Function);
        IRValue *CompileReturn(const ReturnNode *Return);
        IRValue *CompileMethod(const FunctionNode *Method, ClassType* Type);
        IRValue *CompileClass(const ClassNode *Class);
        IRValue *CompileIf(const IfNode *If);
        IRValue *CompileWhile(const WhileNode *While);
        IRValue *CompileFor(const ForNode *For);
        IRValue *CompileBreak();
        IRValue *CompileContinue();

        IRValue *CompileToRValue(const ASTNode* Node)
        {
            IRValue* Value = CompileNode(Node);
            if (!Value) return nullptr;
            return Value->GetRValue(Builder, CContext);
        }

        void DeclareVariable(const std::string& Name, IRValue *Var);
        IRValue *GetVariable(const std::string &Name);

        void EnterScope();
        void ExitScope();

        static bool GetIntegerValue(const ASTNode *Node, Int64 &Num);

        void FillArray(const ArrayNode *Array, llvm::AllocaInst *Alloca);

        template <typename T, typename ...Args_>
        [[nodiscard]] T *Create(Args_&&... Args) { return CompilerArena.Create<T>(std::forward<Args_>(Args)...); }
    };
}

#endif //CVOLT_LLVMCOMPILER_H
