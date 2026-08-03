//
// Created by bohdan on 15.01.26.
//

#ifndef CVOLT_TYPECHECKER_H
#define CVOLT_TYPECHECKER_H

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Errors/TypeError.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Core/TypeDefs/UMap.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "ExprResult.h"
#include "ExprAddress.h"

namespace Volt
{
    class TypeChecker
    {
        struct ScopeEntry
        {
            llvm::StringRef Name;
            ExprAddress* Prev = nullptr;

            ScopeEntry(const llvm::StringRef Name, ExprAddress* Prev)
                : Name(Name), Prev(Prev) {}
        };

        using VariableTable = llvm::StringMap<ExprAddress*>;
        using GlobalVariableTable = llvm::StringMap<ExprAddress*>;

    private:
        CompilationContext& CContext;

        ASTNode*& ASTTree;
        Arena& MainArena;

        BuiltinFunctionTable& BuiltinFuncTable;

        Array<TypeError>& Errors;

        GlobalVariableTable GlobalVariables;

        FunctionTable Functions;
        VariableTable Variables;

        Array<Array<ScopeEntry>> ScopeStack;

        QualType FunctionReturnType;
        bool InFunction = false;

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
        SemaResult *VisitNullPointer(NullPointerNode* NullPtr);
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
        SemaResult *VisitFunctionCall(CallNode *Call);
        SemaResult *VisitMethodCall(CallNode *Call);
        SemaResult *VisitSubscript(SubscriptNode *Subscript);
        SemaResult *VisitExplicitCast(ExplicitCastNode *ECast);
        SemaResult *VisitConstruct(ConstructNode *Construct);
        SemaResult *VisitVariable(VariableNode *Variable);
        SemaResult *VisitVariableConstruct(VariableConstructNode *Construct);
        SemaResult *VisitFunction(FunctionNode *Function);
        void VisitMethod(FunctionNode* Method, ClassType* Type);
        void VisitConstructor(ConstructorNode* Constructor, ClassType* Type);
        SemaResult *VisitClass(ClassNode *Class);
        SemaResult *VisitMemberAccess(MemberAccessNode *MemberAccess);
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

        FunctionCallee* CreateFunction(FunctionNode* Function, llvm::StringRef& Name,
                                       ArgsVector<QualType>& Params, QualType ThisType = {});

        static const FunctionOverload* TryGetFunction(llvm::StringRef Name, llvm::ArrayRef<QualType> Args,
                                               const FunctionTable& FuncTable);
        static const FunctionOverload* TryGetOverload(llvm::ArrayRef<QualType> Args,
                                                      const FuncOverloadVector& Overloads);

        bool ImplicitCastOrError(DataType *&Src, DataType* Dst, size_t Line, size_t Column);

        void EnterScope();
        void ExitScope();

        void DeclareGlobalVariable(VariableNode* Variable);

        void DeclareVariable(llvm::StringRef Name, ExprAddress* Addr);
        ExprAddress* GetVariable(llvm::StringRef Name);

        void DeclareAndAddParams(llvm::ArrayRef<ParamNode*> ParamNodes, ArgsVector<QualType>& ParamTypes);

        friend class LLVMCompiler;
    };
}

#endif //CVOLT_TYPECHECKER_H