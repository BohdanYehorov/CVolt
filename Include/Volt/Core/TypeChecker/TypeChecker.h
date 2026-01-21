//
// Created by bohdan on 15.01.26.
//

#ifndef CVOLT_TYPECHECKER_H
#define CVOLT_TYPECHECKER_H

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Compiler/Types/DataType.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Compiler/Hash/FunctionSignatureHash.h"
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

        std::unordered_map<FunctionSignature, DataType, FunctionSignatureHash> Functions;
        std::unordered_map<std::string, DataType> Variables;

        std::vector<std::vector<TypeScopeEntry>> ScopeStack;

        DataType FunctionReturnType = nullptr;

    public:
        TypeChecker(const Parser& Psr, Arena& MainArena)
            : ASTTree(Psr.GetASTTree()), MainArena(MainArena) {}

    private:
        DataType VisitNode(ASTNode* Node);

        void VisitSequence(SequenceNode* Sequence);
        void VisitBlock(BlockNode* Block);

        DataType VisitInt(IntegerNode *Int);
        DataType VisitFloat(FloatingPointNode* Float);
        DataType VisitBool(BoolNode* Bool);
        DataType VisitChar(CharNode* Char);
        DataType VisitString(StringNode* String);
        DataType VisitIdentifier(IdentifierNode* Identifier);
        DataType VisitSuffix(SuffixOpNode* Suffix);
        DataType VisitPrefix(PrefixOpNode* Prefix);
        DataType VisitUnary(UnaryOpNode* Unary);
        DataType VisitBinary(BinaryOpNode* Binary);
        DataType VisitCall(CallNode* Call);
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

        bool CastToJointType(DataType &Left, DataType &Right, Operator::Type Type);
        static bool ImplicitCast(DataType &Src, DataType Dst);

        void EnterScope();
        void ExitScope();

        void DeclareVariable(const std::string& Name, DataType Type);
        DataType GetVariable(const std::string& Name);
    };
}

#endif //CVOLT_TYPECHECKER_H