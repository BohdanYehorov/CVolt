//
// Created by bohdan on 15.01.26.
//

#ifndef CVOLT_TYPECHECKER_H
#define CVOLT_TYPECHECKER_H

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Core/Errors/TypeError.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Compiler/Types/CompilerTypes.h"
#include "Volt/Core/TypeDefs/UMap.h"
#include "Volt/Core/TypeDefs/VariableTable.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "ExprResult.h"

namespace Volt
{
    struct CTimeScopeEntry
    {
        std::string Name;
        ExprResult* Previous = nullptr;

        CTimeScopeEntry(const std::string& Name, ExprResult* Prev = nullptr)
            : Name(Name), Previous(Prev) {}
    };

    class TypeChecker
    {
    private:
        CompilationContext& CContext;

        ASTNode*& ASTTree;
        Arena& MainArena;

        BuiltinFunctionTable& BuiltinFuncTable;

        Array<TypeError> Errors;

        FunctionTable Functions;
        CTimeVariableTable Variables;

        Array<Array<CTimeScopeEntry>> ScopeStack;

        SmallVec8<std::pair<std::string, QualType>> FunctionParams;
        QualType FunctionReturnType;

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

    private:
        void SendError(TypeErrorKind Kind, size_t Line, size_t Column, Array<std::string>&& Context = {})
        {
            Errors.Emplace(Kind, Line, Column, std::move(Context));
        }

        void SendError(TypeErrorKind Kind, ASTNode* Node, Array<std::string>&& Context = {})
        {
            Errors.Emplace(Kind, Node->Line, Node->Column, std::move(Context));
        }

        ExprResult *VisitNode(ASTNode *Node);

        void VisitSequence(SequenceNode* Sequence);
        void VisitBlock(BlockNode* Block);

        ExprResult *VisitInt(IntegerNode *Int);
        ExprResult *VisitFloat(FloatingPointNode *Float);
        ExprResult *VisitBool(BoolNode *Bool);
        ExprResult *VisitChar(CharNode *Char);
        ExprResult *VisitString(StringNode *String);
        ExprResult *VisitArray(ArrayNode *Array);
        ExprResult *VisitIdentifier(IdentifierNode *Identifier);
        ExprResult *VisitRef(RefNode *Ref);
        ExprResult *VisitUnref(UnrefNode *Unref);
        ExprResult *VisitSuffix(SuffixOpNode *Suffix);
        ExprResult *VisitPrefix(PrefixOpNode *Prefix);
        ExprResult *VisitUnary(UnaryOpNode *Unary);
        ExprResult *VisitBinary(BinaryOpNode *Binary);
        ExprResult *VisitCall(CallNode *Call);
        ExprResult *VisitSubscript(SubscriptNode *Subscript);
        ExprResult *VisitExplicitCast(ExplicitCastNode *ECast);
        ExprResult *VisitVariable(VariableNode *Variable);
        ExprResult *VisitFunction(FunctionNode *Function);
        ExprResult *VisitIf(IfNode *If);
        ExprResult *VisitWhile(WhileNode *While);
        ExprResult *VisitFor(ForNode *For);
        ExprResult *VisitReturn(ReturnNode *Return);

        QualType VisitType(DataTypeNodeBase *Type);
        ExprResult* GetLValue(ASTNode* Node, bool IgnoreConstants = false);

        static QualType GetNotReferenceType(QualType Type);

        template <typename MapT>
        MapT::const_iterator TryGetOverload(const FunctionSignature& Signature, const MapT& Map);

        [[nodiscard]] static bool CanCastPointers(PointerType* Src, PointerType* Dst);
        bool ImplicitCastOrError(DataType *&Src, DataType* Dst, size_t Line, size_t Column);

        void EnterScope();
        void ExitScope();

        void DeclareVariable(const std::string& Name, QualType Type);
        QualType GetVariable(const std::string& Name);

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