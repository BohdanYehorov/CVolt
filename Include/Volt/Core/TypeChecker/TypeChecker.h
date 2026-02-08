//
// Created by bohdan on 15.01.26.
//

#ifndef CVOLT_TYPECHECKER_H
#define CVOLT_TYPECHECKER_H

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Core/Types/DataTypeUtils.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Core/Errors/TypeError.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Compiler/Types/CompilerTypes.h"
#include "Volt/Compiler/CompileTime/CTimeValue.h"
#include "Volt/Core/TypeDefs/FunctionTable.h"
#include "Volt/Core/TypeDefs/VariableTable.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>

namespace Volt
{
    // struct TypeScopeEntry
    // {
    //     std::string Name;
    //     DataType Previous = nullptr;
    // };

    class TypeChecker
    {
    private:
        static llvm::DenseMap<TypeCategory, llvm::DenseSet<TypeCategory>> ImplicitCastTypes;

    private:
        CompilationContext& CContext;

        ASTNode*& ASTTree;
        Arena& MainArena;

        // BuilderBase& Builder;

        BuiltinFunctionTable& BuiltinFuncTable;

        std::vector<TypeError> Errors;

        FunctionTable Functions;
        VariableTable Variables;

        std::vector<std::vector<ScopeEntry>> ScopeStack;

        SmallVec8<std::pair<std::string, DataType*>> FunctionParams;
        DataType* FunctionReturnType = nullptr;

    public:
        // TypeChecker(const Parser& Psr, Arena& MainArena, BuilderBase& Builder, BuiltinFunctionTable& BuiltinFuncTable)
        //     : ASTTree(Psr.GetASTTree()), MainArena(MainArena),
        //     Builder(Builder), BuiltinFuncTable(BuiltinFuncTable) {}

        TypeChecker(CompilationContext& CContext, BuiltinFunctionTable& BuiltinFuncTable)
            : CContext(CContext), ASTTree(CContext.ASTTree),
            MainArena(CContext.MainArena), BuiltinFuncTable(BuiltinFuncTable) {}

        void Check() { VisitNode(ASTTree); }
        [[nodiscard]] bool HasErrors() const { return !Errors.empty(); }
        void WriteErrors(std::ostream& Os) const;
        bool PrintErrors() const
        {
            WriteErrors(std::cout);
            return HasErrors();
        }

        [[nodiscard]] FunctionTable& GetFunctions() { return Functions; }
        [[nodiscard]] VariableTable& GetVariables() { return Variables; }
        [[nodiscard]] ASTNode* GetASTTree() const { return ASTTree; }
        [[nodiscard]] BuiltinFunctionTable& GetBuiltinFunctionTable() const { return BuiltinFuncTable; }

    private:
        void SendError(TypeErrorKind Kind, size_t Line, size_t Column, std::vector<std::string>&& Context = {})
        {
            Errors.emplace_back(Kind, Line, Column, std::move(Context));
        }

        void SendError(TypeErrorKind Kind, ASTNode* Node, std::vector<std::string>&& Context = {})
        {
            Errors.emplace_back(Kind, Node->Line, Node->Column, std::move(Context));
        }

        CTimeValue *VisitNode(ASTNode *Node);

        void VisitSequence(SequenceNode* Sequence);
        void VisitBlock(BlockNode* Block);

        CTimeValue *VisitInt(IntegerNode *Int);
        CTimeValue *VisitFloat(FloatingPointNode *Float);
        CTimeValue *VisitBool(BoolNode *Bool);
        CTimeValue *VisitChar(CharNode *Char);
        CTimeValue *VisitString(StringNode *String);
        CTimeValue *VisitArray(ArrayNode *Array);
        CTimeValue *VisitIdentifier(IdentifierNode *Identifier);
        CTimeValue *VisitRef(RefNode *Ref);
        CTimeValue *VisitSuffix(SuffixOpNode *Suffix);
        CTimeValue *VisitPrefix(PrefixOpNode *Prefix);
        CTimeValue *VisitUnary(UnaryOpNode *Unary);
        CTimeValue *VisitComparison(ComparisonNode *Comparison);
        CTimeValue *VisitBinary(BinaryOpNode *Binary);
        CTimeValue *VisitCall(CallNode *Call);
        CTimeValue *VisitSubscript(SubscriptNode *Subscript);
        CTimeValue *VisitVariable(VariableNode *Variable);
        CTimeValue *VisitFunction(FunctionNode *Function);
        CTimeValue *VisitIf(IfNode *If);
        CTimeValue *VisitWhile(WhileNode *While);
        CTimeValue *VisitFor(ForNode *For);
        CTimeValue *VisitReturn(ReturnNode *Return);

        DataType *VisitType(DataTypeNodeBase *Type);

        [[nodiscard]] bool CanImplicitCast(DataType* Src, DataType* Dst) const;
        [[nodiscard]] bool CanCastArithmetic(DataType* Left, DataType* Right, OperatorType Type) const;
        [[nodiscard]] bool CanCastComparison(DataType* Left, DataType* Right, OperatorType Type) const;
        [[nodiscard]] bool CanCastLogical(DataType* Left, DataType* Right, OperatorType Type) const;
        [[nodiscard]] bool CanCastBitwise(DataType* Left, DataType* Right, OperatorType Type) const;
        [[nodiscard]] bool CanCastAssignment(DataType* Left, DataType* Right, OperatorType Type) const;
        [[nodiscard]] bool CanCastToJointType(DataType* Left, DataType* Right, OperatorType Type) const;

        bool CastToJointType(DataType *&Left, DataType *&Right, OperatorType Type, size_t Line, size_t Column);
        static bool ImplicitCast(DataType *&Src, DataType *Dst);
        bool ImplicitCastOrError(DataType *&Src, DataType* Dst, size_t Line, size_t Column);

        static bool ImplicitCast(CTimeValue *Src, DataType* DstType);
        bool CastToJointType(CTimeValue *Left, CTimeValue *Right, OperatorType Type, size_t Line, size_t Column);

        void EnterScope();
        void ExitScope();

        void DeclareVariable(const std::string& Name, DataType* Type);
        DataType* GetVariable(const std::string& Name);

        CTimeValue *CalculateUnary(CTimeValue *Operand, OperatorType Type) const;
        CTimeValue *CalculateBinary(CTimeValue *Left, CTimeValue *Right, OperatorType Type) const;

        friend class LLVMCompiler;
    };
}

#endif //CVOLT_TYPECHECKER_H