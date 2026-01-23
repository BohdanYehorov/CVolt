//
// Created by bohdan on 15.01.26.
//

#ifndef CVOLT_TYPECHECKER_H
#define CVOLT_TYPECHECKER_H

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/Types/DataType.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Core/Errors/Errors.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/Types/TypeDefs.h"
#include "Volt/Compiler/Types/CompilerTypes.h"
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>

namespace Volt
{
    struct TypeScopeEntry
    {
        std::string Name;
        DataType Previous = nullptr;
    };

    class TypeChecker
    {
    private:
        static llvm::DenseMap<TypeCategory, llvm::DenseSet<TypeCategory>> ImplicitCastTypes;

    private:
        ASTNode* ASTTree;
        Arena& MainArena;
        BuiltinFunctionTable& BuiltinFuncTable;

        std::vector<TypeError> Errors;

        FunctionTable Functions;
        VariableTable Variables;

        std::vector<std::vector<ScopeEntry>> ScopeStack;

        llvm::SmallVector<std::pair<std::string, DataType>, 8> FunctionParams;
        DataType FunctionReturnType = nullptr;

    public:
        TypeChecker(const Parser& Psr, Arena& MainArena, BuiltinFunctionTable& BuiltinFuncTable)
            : ASTTree(Psr.GetASTTree()), MainArena(MainArena), BuiltinFuncTable(BuiltinFuncTable) {}

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

        DataType VisitNode(ASTNode* Node);

        void VisitSequence(SequenceNode* Sequence);
        void VisitBlock(BlockNode* Block);

        DataType VisitInt(IntegerNode *Int);
        DataType VisitFloat(FloatingPointNode* Float);
        DataType VisitBool(BoolNode* Bool);
        DataType VisitChar(CharNode* Char);
        DataType VisitString(StringNode* String);
        DataType VisitArray(ArrayNode *Array);
        DataType VisitIdentifier(IdentifierNode* Identifier);
        DataType VisitRef(RefNode* Ref);
        DataType VisitSuffix(SuffixOpNode* Suffix);
        DataType VisitPrefix(PrefixOpNode* Prefix);
        DataType VisitUnary(UnaryOpNode* Unary);
        DataType VisitComparison(ComparisonNode *Comparison);
        DataType VisitBinary(BinaryOpNode* Binary);
        DataType VisitCall(CallNode* Call);
        DataType VisitSubscript(SubscriptNode* Subscript);
        DataType VisitVariable(VariableNode* Variable);
        DataType VisitFunction(FunctionNode* Function);
        DataType VisitIf(IfNode* If);
        DataType VisitWhile(WhileNode* While);
        DataType VisitFor(ForNode* For);
        DataType VisitReturn(ReturnNode* Return);

        [[nodiscard]] bool CanImplicitCast(DataType Src, DataType Dst) const;
        [[nodiscard]] bool CanCastArithmetic(DataType Left, DataType Right, Operator::Type Type) const;
        [[nodiscard]] bool CanCastComparison(DataType Left, DataType Right, Operator::Type Type) const;
        [[nodiscard]] bool CanCastLogical(DataType Left, DataType Right, Operator::Type Type) const;
        [[nodiscard]] bool CanCastBitwise(DataType Left, DataType Right, Operator::Type Type) const;
        [[nodiscard]] bool CanCastAssignment(DataType Left, DataType Right, Operator::Type Type) const;
        [[nodiscard]] bool CanCastToJointType(DataType Left, DataType Right, Operator::Type Type) const;

        bool CastToJointType(DataType &Left, DataType &Right, Operator::Type Type, size_t Line, size_t Column);
        static bool ImplicitCast(DataType &Src, DataType Dst);
        bool ImplicitCastOrError(DataType& Src, DataType Dst, size_t Line, size_t Column);

        void EnterScope();
        void ExitScope();

        void DeclareVariable(const std::string& Name, DataType Type);
        DataType GetVariable(const std::string& Name);
    };
}

#endif //CVOLT_TYPECHECKER_H