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
    class TypeChecker
    {
    private:
        static llvm::DenseMap<TypeCategory, llvm::DenseSet<TypeCategory>> ImplicitCastTypes;

    private:
        CompilationContext& CContext;

        ASTNode*& ASTTree;
        Arena& MainArena;

        BuiltinFunctionTable& BuiltinFuncTable;

        Array<TypeError> Errors;

        FunctionTable Functions;
        VariableTable Variables;

        Array<Array<ScopeEntry>> ScopeStack;

        SmallVec8<std::pair<std::string, DataType*>> FunctionParams;
        DataType* FunctionReturnType = nullptr;

    public:
        TypeChecker(CompilationContext& CContext, BuiltinFunctionTable& BuiltinFuncTable)
            : CContext(CContext), ASTTree(CContext.ASTTree),
            MainArena(CContext.MainArena), BuiltinFuncTable(BuiltinFuncTable) {}

        void Check() { VisitNode(ASTTree); }
        [[nodiscard]] bool HasErrors() const { return !Errors.Empty(); }
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
        void SendError(TypeErrorKind Kind, size_t Line, size_t Column, Array<std::string>&& Context = {})
        {
            Errors.Emplace(Kind, Line, Column, std::move(Context));
        }

        void SendError(TypeErrorKind Kind, ASTNode* Node, Array<std::string>&& Context = {})
        {
            Errors.Emplace(Kind, Node->Line, Node->Column, std::move(Context));
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

        template <typename MapT>
        MapT::const_iterator TryGetOverload(const FunctionSignature& Signature, const MapT& Map);

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

    template<typename MapT>
    MapT::const_iterator TypeChecker::TryGetOverload(const FunctionSignature &Signature, const MapT &Map)
    {
        if (auto Iter = Map.find(Signature); Iter != Map.end())
            return Iter;

        size_t ArgsCount = Signature.Params.size();
        llvm::ArrayRef<DataType*> ArgTypes = Signature.Params;

        size_t MinCasts = ArgsCount;
        int BestRank = std::numeric_limits<int>::max();
        auto BestIt = Map.end();
        for (auto Iter = Map.begin(); Iter != Map.end(); ++Iter)
        {
            const FunctionSignature& CandidateSignature = Iter->first;

            if (CandidateSignature.Name != Signature.Name ||
                CandidateSignature.Params.size() != ArgTypes.size()) continue;

            int RankDiff = 0;
            size_t Casts = 0;
            bool Valid = true;
            for (size_t i = 0; i < ArgsCount; i++)
            {
                DataType* CandidateArgType = CandidateSignature.Params[i];
                DataType* ArgType = ArgTypes[i];

                if (!CanImplicitCast(ArgType, CandidateArgType))
                {
                    Valid = false;
                    break;
                }

                if (ArgType != CandidateArgType)
                    Casts++;

                RankDiff += std::abs(
                    DataTypeUtils::GetTypeRank(CandidateArgType) - DataTypeUtils::GetTypeRank(ArgType));
            }

            if (!Valid) continue;

            if (BestIt == Map.end() || Casts < MinCasts || (Casts == MinCasts && RankDiff < BestRank))
            {
                MinCasts = Casts;
                BestRank = RankDiff;
                BestIt = Iter;
            }
        }

        return BestIt;
    }
}

#endif //CVOLT_TYPECHECKER_H