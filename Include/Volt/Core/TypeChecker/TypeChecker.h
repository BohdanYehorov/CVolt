//
// Created by bohdan on 15.01.26.
//

#ifndef CVOLT_TYPECHECKER_H
#define CVOLT_TYPECHECKER_H

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Functions/FunctionSignature.h"
#include "Volt/Core/Errors/TypeError.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Compiler/Types/CompilerTypes.h"
#include "Volt/Core/TypeDefs/UMap.h"
#include "Volt/Core/TypeDefs/VariableTable.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "ExprResult.h"
#include "ExprAddress.h"

namespace Volt
{
    class TypeChecker
    {
        struct ScopeEntry
        {
            std::string Name;
            ExprAddress* Prev = nullptr;

            ScopeEntry(const std::string& Name, ExprAddress* Prev)
                : Name(Name), Prev(Prev) {}
        };

    private:
        CompilationContext& CContext;

        ASTNode*& ASTTree;
        Arena& MainArena;

        BuiltinFunctionTable& BuiltinFuncTable;

        Array<TypeError>& Errors;

        FunctionTable Functions;
        CTimeVariableTable Variables;

        Array<Array<ScopeEntry>> ScopeStack;

        SmallVec8<std::pair<std::string, QualType>> FunctionParams;
        QualType FunctionReturnType;

    public:
        TypeChecker(CompilationContext& CContext, BuiltinFunctionTable& BuiltinFuncTable)
            : CContext(CContext), ASTTree(CContext.ASTTree),
            MainArena(CContext.MainArena), BuiltinFuncTable(BuiltinFuncTable),
            Errors(CContext.TypeErrors) {}

        void Check()
        {
            if (CContext.HasErrors()) return;
            VisitNode(ASTTree);
        }

    private:
        void SendError(TypeErrorKind Kind, size_t Line, size_t Column, Array<std::string>&& Context = {})
        {
            Errors.Emplace(Kind, Line, Column, std::move(Context));
        }

        void SendError(TypeErrorKind Kind, ASTNode* Node, Array<std::string>&& Context = {})
        {
            Errors.Emplace(Kind, Node->Line, Node->Column, std::move(Context));
        }

        SemaResult *VisitNode(ASTNode *Node);

        void VisitSequence(SequenceNode* Sequence);
        void VisitBlock(BlockNode* Block);

        SemaResult *VisitInt(IntegerNode *Int);
        SemaResult *VisitFloat(FloatingPointNode *Float);
        SemaResult *VisitBool(BoolNode *Bool);
        SemaResult *VisitChar(CharNode *Char);
        SemaResult *VisitString(StringNode *String);
        SemaResult *VisitArray(ArrayNode *Array);
        SemaResult *VisitIdentifier(IdentifierNode *Identifier);
        SemaResult *VisitRef(RefNode *Ref);
        SemaResult *VisitUnref(UnrefNode *Unref);
        SemaResult *VisitSuffix(SuffixOpNode *Suffix);
        SemaResult *VisitPrefix(PrefixOpNode *Prefix);
        SemaResult *VisitUnary(UnaryOpNode *Unary);
        SemaResult *VisitAssignment(AssignmentNode* Assignment);
        SemaResult *VisitComparison(ComparisonNode* Comparison);
        SemaResult *VisitBinary(BinaryOpNode *Binary);
        SemaResult *VisitCall(CallNode *Call);
        SemaResult *VisitSubscript(SubscriptNode *Subscript);
        SemaResult *VisitExplicitCast(ExplicitCastNode *ECast);
        SemaResult *VisitVariable(VariableNode *Variable);
        SemaResult *VisitFunction(FunctionNode *Function);
        SemaResult *VisitIf(IfNode *If);
        SemaResult *VisitWhile(WhileNode *While);
        SemaResult *VisitFor(ForNode *For);
        SemaResult *VisitReturn(ReturnNode *Return);

        QualType VisitType(DataTypeNodeBase *Type);

        static ExprResult* GetRValue(SemaResult* Value);
        ExprResult* VisitToRValue(ASTNode* Node)
        {
            SemaResult* Result = VisitNode(Node);
            if (!Result) return nullptr;
            return GetRValue(Result);
        }

        ExprAddress* VisitToLValue(ASTNode* Node);
        ExprAddress* VisitToLValueAndCheckConst(ASTNode* Node);

        static QualType GetNotReferenceType(QualType Type);

        template <typename MapT>
        MapT::const_iterator TryGetOverload(const FunctionSignature& Signature, const MapT& Map);

        [[nodiscard]] static bool CanCastPointers(PointerType* Src, PointerType* Dst);
        bool ImplicitCastOrError(DataType *&Src, DataType* Dst, size_t Line, size_t Column);

        void EnterScope();
        void ExitScope();

        void DeclareVariable(const std::string& Name, ExprAddress* Addr);
        ExprAddress* GetVariable(const std::string& Name);

        friend class LLVMCompiler;
    };

    template<typename MapT>
    MapT::const_iterator TypeChecker::TryGetOverload(const FunctionSignature &Signature, const MapT &Map)
    {
        if (auto Iter = Map.find(Signature); Iter != Map.end())
            return Iter;

        size_t ArgsCount = Signature.Params.size();
        llvm::ArrayRef<QualType> ArgTypes = Signature.Params;

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
                QualType CandidateArgType = CandidateSignature.Params[i];
                QualType ArgType = ArgTypes[i];

                if (auto RefType = CandidateArgType.CastAs<ReferenceType>())
                {
                    if (RefType->CanBind(ArgType))
                        continue;

                    Valid = false;
                    break;
                }

                if (!ArgType.ImplicitCast(CandidateArgType))
                {
                    Valid = false;
                    break;
                }

                if (ArgType != CandidateArgType)
                    Casts++;

                RankDiff += std::abs(
                    CandidateArgType->GetRank() - ArgType->GetRank());
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