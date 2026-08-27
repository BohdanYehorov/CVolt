//
// Created by bohdan on 15.01.26.
//

#include "Volt/Core/TypeChecker/TypeChecker.h"

#include "Volt/Core/Functions/MethodCallee.h"
#include "Volt/Core/TypeChecker/ExprAddress.h"
#include "Volt/Support/ErrorHandling.h"

namespace Volt
{
    void TypeChecker::Visit(ASTNode *Node)
    {
        VisitNode(Node);
        VisitClassAndMethodDecls();
        VisitFunctionBodies();
    }

    SemaResult* TypeChecker::VisitNode(ASTNode *Node)
    {
        if (!Node) return nullptr;

        switch (Node->GetNodeKind())
        {
            case NodeKind::SequenceNode:
                VisitSequence(StaticCast<SequenceNode>(Node));
                return nullptr;
            case NodeKind::BlockNode:
                VisitBlock(StaticCast<BlockNode>(Node));
                return nullptr;
            case NodeKind::IntegerNode:
                return VisitInt(StaticCast<IntegerNode>(Node));
            case NodeKind::FloatingPointNode:
                return VisitFloat(StaticCast<FloatingPointNode>(Node));
            case NodeKind::BoolNode:
                return VisitBool(StaticCast<BoolNode>(Node));
            case NodeKind::CharNode:
                return VisitChar(StaticCast<CharNode>(Node));
            case NodeKind::StringNode:
                return VisitString(StaticCast<StringNode>(Node));
            case NodeKind::ArrayNode:
                return VisitArray(StaticCast<ArrayNode>(Node));
            case NodeKind::NullPointerNode:
                return VisitNullPointer(StaticCast<NullPointerNode>(Node));
            case NodeKind::IdentifierNode:
                return VisitIdentifier(StaticCast<IdentifierNode>(Node));
            case NodeKind::RefNode:
                return VisitRef(StaticCast<RefNode>(Node));
            case NodeKind::UnrefNode:
                return VisitUnref(StaticCast<UnrefNode>(Node));
            case NodeKind::SuffixOpNode:
                return VisitSuffix(StaticCast<SuffixOpNode>(Node));
            case NodeKind::PrefixOpNode:
                return VisitPrefix(StaticCast<PrefixOpNode>(Node));
            case NodeKind::UnaryOpNode:
                return VisitUnary(StaticCast<UnaryOpNode>(Node));
            case NodeKind::AssignmentNode:
                return VisitAssignment(StaticCast<AssignmentNode>(Node));
            case NodeKind::ComparisonNode:
                return VisitComparison(StaticCast<ComparisonNode>(Node));
            case NodeKind::BinaryOpNode:
            case NodeKind::LogicalNode:
                return VisitBinary(StaticCast<BinaryOpNode>(Node));
            case NodeKind::CallNode:
                return VisitCall(StaticCast<CallNode>(Node));
            case NodeKind::SubscriptNode:
                return VisitSubscript(StaticCast<SubscriptNode>(Node));
            case NodeKind::ExplicitCastNode:
                return VisitExplicitCast(StaticCast<ExplicitCastNode>(Node));
            case NodeKind::SizeOfNode:
                return VisitSizeOf(StaticCast<SizeOfNode>(Node));
            case NodeKind::AlignOfNode:
                return VisitAlignOf(StaticCast<AlignOfNode>(Node));
            case NodeKind::VariableNode:
                return VisitVariable(StaticCast<VariableNode>(Node));
            case NodeKind::VariableConstructNode:
                return VisitVariableConstruct(StaticCast<VariableConstructNode>(Node));
            case NodeKind::FunctionNode:
                return VisitFunction(StaticCast<FunctionNode>(Node));
            case NodeKind::ClassNode:
                return VisitClass(StaticCast<ClassNode>(Node));
            case NodeKind::MemberAccessNode:
                return VisitMemberAccess(StaticCast<MemberAccessNode>(Node));
            case NodeKind::IfNode:
                return VisitIf(StaticCast<IfNode>(Node));
            case NodeKind::WhileNode:
                return VisitWhile(StaticCast<WhileNode>(Node));
            case NodeKind::ForNode:
                return VisitFor(StaticCast<ForNode>(Node));
            case NodeKind::ReturnNode:
                return VisitReturn(StaticCast<ReturnNode>(Node));
            case NodeKind::BreakNode:
            case NodeKind::ContinueNode:
                return nullptr;
        }

        return nullptr;
    }

    void TypeChecker::VisitSequence(SequenceNode *Sequence)
    {
        for (auto Statement : Sequence->Statements)
            VisitNode(Statement);
    }

    void TypeChecker::VisitBlock(BlockNode *Block)
    {
        Variables.EnterScope();
        for (auto Statement : Block->Statements)
            VisitNode(Statement);
        Variables.ExitScope();
    }

    SemaResult *TypeChecker::VisitInt(IntegerNode *Int)
    {
        Int->CompileTimeValue = ExprResult::CreateInteger(QualType(CContext.GetIntegerType(
            Int->BitWidth, Int->IsSigned), QualType::CONST), Int->Value, MainArena);
        return Int->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitFloat(FloatingPointNode *Float)
    {
        Float->CompileTimeValue = ExprResult::CreateFloat(
            QualType(CContext.GetFPType(Float->BitWidth), QualType::CONST), Float->Value, MainArena);
        return Float->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitBool(BoolNode *Bool)
    {
        Bool->CompileTimeValue = ExprResult::CreateBool(
            QualType(CContext.GetBoolType(), QualType::CONST), Bool->Value, MainArena);
        return Bool->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitChar(CharNode *Char)
    {
        Char->CompileTimeValue = ExprResult::CreateChar(
            QualType(CContext.GetCharType(), QualType::CONST), Char->Value, MainArena);
        return Char->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitString(StringNode *String)
    {
        String->CompileTimeValue = ExprResult::CreateEmpty(
            QualType(CContext.GetPointerType(QualType(CContext.GetCharType(),
                QualType::CONST)), QualType::CONST), MainArena);
        return String->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitArray(ArrayNode *Array)
    {
        llvm::ArrayRef<ASTNode*> Elements = Array->Elements;

        if (Elements.empty())
            return nullptr;

        QualType ElementsType;
        bool HasErrors = false;
        for (auto El : Elements)
        {
            ExprResult* ElValue = VisitToRValue(El);
            if (!ElValue)
                return nullptr;

            QualType ElType = ElValue->GetType();
            if (!ElType)
                return nullptr;

            if (!ElementsType)
                ElementsType = ElType;
            else if (ElementsType != ElType)
            {
                SendError(TypeErrorKind::ArrayElementTypeMismatch,
                    El, { ElementsType->ToString(), ElType->ToString() });
                HasErrors = true;
            }
        }

        if (HasErrors)
            return nullptr;

        Array->CompileTimeValue = ExprResult::CreateEmpty(QualType(
            CContext.GetArrayType(ElementsType, Elements.size()), QualType::CONST), MainArena);
        return Array->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitNullPointer(NullPointerNode *NullPtr)
    {
        NullPtr->CompileTimeValue = ExprResult::CreateEmpty(CContext.GetNullPointerType(), MainArena);
        return NullPtr->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitIdentifier(IdentifierNode *Identifier)
    {
        ExprAddress* VarAddr = Variables.GetVariable(Identifier->Value);
        if (VarAddr)
        {
            Identifier->CompileTimeValue = VarAddr->GetValue();
            return VarAddr;
        }

        if (auto Iter = GlobalVariables.find(Identifier->Value); Iter != GlobalVariables.end())
        {
            Identifier->CompileTimeValue = Iter->second;
            return Iter->second;
        }

        SendError(TypeErrorKind::UndefinedVariable, Identifier, { Identifier->Value.str() });
        return nullptr;
    }

    SemaResult *TypeChecker::VisitRef(RefNode *Ref)
    {
        ExprAddress* RefAddr = VisitToLValue(Ref->Target);
        if (!RefAddr) return nullptr;

        if (RefAddr->IsEmpty())
            Ref->CompileTimeValue = ExprResult::CreateEmpty(
                CContext.GetPointerType(RefAddr->GetType()), MainArena);
        else
            Ref->CompileTimeValue = ExprResult::CreatePointer(
                CContext.GetPointerType(RefAddr->GetType()), RefAddr, MainArena);

        return Ref->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitUnref(UnrefNode *Unref)
    {
        ExprResult* UnrefValue = VisitToRValue(Unref->Target);
        if (!UnrefValue) return nullptr;

        QualType Type = UnrefValue->GetType();
        if (!Type) return nullptr;

        if (auto PtrType = Cast<PointerType>(Type.GetType()))
        {
            if (UnrefValue->IsEmpty())
                Unref->CompileTimeValue = MainArena.Create<ExprAddress>(
                    ExprResult::CreateEmpty(PtrType->GetBaseType(), MainArena));
            else
                Unref->CompileTimeValue = UnrefValue->GetPointer();

            return Unref->CompileTimeValue;
        }

        return nullptr;
    }

    SemaResult *TypeChecker::VisitSuffix(SuffixOpNode *Suffix)
    {
        ExprAddress* OperandAddr = VisitToLValueAndCheckConst(Suffix->Operand);
        if (!OperandAddr) return nullptr;

        ExprResult* Temp = OperandAddr->GetValue();

        OperatorType AssignmentType;
        switch (Suffix->Type)
        {
            case OperatorType::Inc: AssignmentType = OperatorType::AddAssign; break;
            case OperatorType::Dec: AssignmentType = OperatorType::SubAssign; break;
            default: VoltUnreachable("Unknown suffix operator");
        }

        OperandAddr->CreateAssignment(ExprResult::CreateFromType(
                OperandAddr->GetType(), 1, MainArena), AssignmentType, CContext);

        return Temp;
    }

    SemaResult *TypeChecker::VisitPrefix(PrefixOpNode *Prefix)
    {
        ExprAddress* OperandAddr = VisitToLValueAndCheckConst(Prefix->Operand);
        if (!OperandAddr) return nullptr;

        OperatorType AssignmentType;
        switch (Prefix->Type)
        {
            case OperatorType::Inc: AssignmentType = OperatorType::AddAssign; break;
            case OperatorType::Dec: AssignmentType = OperatorType::SubAssign; break;
            default: VoltUnreachable("Unknown prefix operator");
        }

        OperandAddr->CreateAssignment(ExprResult::CreateFromType(
                OperandAddr->GetType(), 1, MainArena), AssignmentType, CContext);

        return OperandAddr;
    }

    SemaResult *TypeChecker::VisitUnary(UnaryOpNode *Unary)
    {
        using enum OperatorType;

        ExprResult* Operand = VisitToRValue(Unary->Operand);
        if (!Operand)
            return nullptr;

        if (Operand->IsEmpty())
        {
            QualType OperandType = Operand->GetType();
            TypeError Err(Unary->Line, Unary->Column);
            if (auto Type = Operator::ResolveUnary(OperandType, Unary->Type, Err))
            {
                Unary->CompileTimeValue = ExprResult::CreateEmpty(Type, MainArena);;
                return Unary->CompileTimeValue;
            }

            Errors.Add(Err);
            return nullptr;
        }

        switch (Unary->Type)
        {
            case UnPlus:     break;
            case UnMinus:    Operand = Operand->CreateNeg(CContext); break;
            case BitNot:     Operand = Operand->CreateBitNot(CContext); break;
            case LogicalNot: Operand = Operand->CreateNot(CContext); break;
            default: VoltUnreachable("Unknown unary operator");
        }

        Unary->CompileTimeValue = Operand;
        return Operand;
    }

    SemaResult *TypeChecker::VisitAssignment(AssignmentNode *Assignment)
    {
        ExprAddress* LeftAddr = VisitToLValueAndCheckConst(Assignment->Left);
        ExprResult* Right = VisitToRValue(Assignment->Right);

        if (!Right || !LeftAddr) return nullptr;

        QualType RightType = Right->GetType();
        Right = Right->ImplicitCast(LeftAddr->GetType().GetNotReferenceType(), CContext);
        if (!Right)
        {
            SendError(TypeErrorKind::IncompatibleTypes, Assignment,
                { RightType.ToString(), LeftAddr->GetType().ToString() });
            return nullptr;
        }

        Assignment->CompileTimeValue = LeftAddr->CreateAssignment(Right, Assignment->Type, CContext);
        Assignment->LeftOperandType = LeftAddr->GetType().GetType();
        Assignment->RightOperandType = Right->GetType().GetType();
        return Assignment->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitComparison(ComparisonNode *Comparison)
    {
        ExprResult* Left = VisitToRValue(Comparison->Left);
        ExprResult* Right = VisitToRValue(Comparison->Right);
        if (!Left || !Right)
            return nullptr;

        QualType LeftType = Left->GetType();
        QualType RightType = Right->GetType();

        TypeError Err(Comparison->Line, Comparison->Column);
        QualType Type = Operator::ResolveComparison(
            LeftType, RightType, Comparison->Type, Err, CContext);
        if (!Type)
        {
            Errors.Add(std::move(Err));
            return nullptr;
        }

        Left = Left->ImplicitCast(LeftType, CContext);
        Right = Right->ImplicitCast(RightType, CContext);

        Comparison->LeftOperandType = Left->GetType().GetType();
        Comparison->RightOperandType = Right->GetType().GetType();

        if (Left->IsEmpty() || Right->IsEmpty())
        {
            Comparison->CompileTimeValue = ExprResult::CreateEmpty(Type, MainArena);
            return Comparison->CompileTimeValue;
        }

        Comparison->CompileTimeValue = Left->CreateCmp(Right, Comparison->Type, CContext);
        return Comparison->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitBinary(BinaryOpNode *Binary)
    {
        using enum OperatorType;

        ExprResult* Left = VisitToRValue(Binary->Left);
        ExprResult* Right = VisitToRValue(Binary->Right);
        if (!Left || !Right)
            return nullptr;

        QualType LeftType = Left->GetType();
        QualType RightType = Right->GetType();

        TypeError Err(Binary->Line, Binary->Column);
        QualType Type = Operator::ResolveBinary(LeftType, RightType, Binary->Type, Err, CContext);
        if (!Type)
        {
            Errors.Add(std::move(Err));
            return nullptr;
        }

        Left = Left->ImplicitCast(LeftType, CContext);
        Right = Right->ImplicitCast(RightType, CContext);

        Binary->LeftOperandType = Left->GetType().GetType();
        Binary->RightOperandType = Right->GetType().GetType();

        if (Left->IsEmpty() || Right->IsEmpty())
        {
            Binary->CompileTimeValue = ExprResult::CreateEmpty(Type, MainArena);
            return Binary->CompileTimeValue;
        }

        ExprResult* Result;
        switch (Binary->Type)
        {
            case Add:     Result = Left->CreateAdd(Right, CContext); break;
            case Sub:     Result = Left->CreateSub(Right, CContext); break;
            case Mul:     Result = Left->CreateMul(Right, CContext); break;
            case Div:     Result = Left->CreateDiv(Right, CContext); break;
            case Mod:     Result = Left->CreateMod(Right, CContext); break;
            case BitAnd:  Result = Left->CreateBitAnd(Right, CContext); break;
            case BitOr:   Result = Left->CreateBitOr(Right, CContext); break;
            case BitXor:  Result = Left->CreateBitXor(Right, CContext); break;
            case RShift:  Result = Left->CreateBitRShift(Right, CContext); break;
            case LShift:  Result = Left->CreateBitLShift(Right, CContext); break;
            default: VoltUnreachable("Unknown binary operator");
        }

        Binary->CompileTimeValue = Result;
        return Result;
    }

    SemaResult *TypeChecker::VisitCall(CallNode *Call)
    {
        if (SemaResult* Res = VisitConstruct(Call))
            return Res;

        if (SemaResult* Res = VisitFunctionCall(Call))
            return Res;

        if (SemaResult* Res = VisitMethodCall(Call))
            return Res;

        SendError(TypeErrorKind::InvalidCalleeType, Call->Callee);
        return nullptr;
    }

    SemaResult *TypeChecker::VisitFunctionCall(CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            llvm::StringRef Name = Identifier->Value;
            size_t ArgsCount = Call->Arguments.size();
            ArgsVector<QualType> ArgTypes;
            ArgTypes.reserve(ArgsCount);

            for (auto Arg : Call->Arguments)
            {
                ExprResult* ArgValue = VisitToRValue(Arg);
                if (!ArgValue)
                    return nullptr;

                QualType ArgType = ArgValue->GetType();
                if (!ArgType)
                    return nullptr;

                ArgTypes.push_back(ArgType);
            }

            FunctionTable::OverloadResult FunctionOverloadRes = Functions.FindBestFunctionOverload(Name, ArgTypes);
            if (FunctionOverloadRes.Kind != FunctionTable::OverloadResult::NotAvailable)
            {
                const FunctionOverload* Overload = FunctionOverloadRes.FirstOverload;
                Call->ResolvedCallee = Overload->Callee;

                for (size_t i = 0; i < ArgsCount; i++)
                    Call->Arguments[i]->ExpectedType = Overload->Args[i].GetType();

                return ExprResult::CreateEmpty(Overload->Callee->FuncType->GetReturnType(), MainArena);
            }

            BuiltinFuncTable::OverloadResult BuiltinFuncOverloadRes =
                BuiltinFuncTable.GetFunctionTable().FindBestFunctionOverload(Name, ArgTypes);
            if (BuiltinFuncOverloadRes.Kind != BuiltinFuncTable::OverloadResult::NotAvailable)
            {
                const BuiltinFunctionOverload* Overload = BuiltinFuncOverloadRes.FirstOverload;
                Call->ResolvedCallee = Overload->Callee;

                for (size_t i = 0; i < ArgsCount; i++)
                    Call->Arguments[i]->ExpectedType = Overload->Args[i].GetType();

                return ExprResult::CreateEmpty(Overload->Callee->FuncType->GetReturnType(), MainArena);
            }

            Array<std::string> ErrorContext = { Name.str() };
            ErrorContext.Reserve(ArgsCount);

            for (auto Arg : ArgTypes)
                ErrorContext.Add(Arg->ToString());

            SendError(TypeErrorKind::NoFunctionOverload, Call->Callee, std::move(ErrorContext));
            return nullptr;
        }

        return nullptr;
    }

    SemaResult *TypeChecker::VisitMethodCall(CallNode *Call)
    {
        if (auto MemberAccess = Cast<MemberAccessNode>(Call->Callee))
        {
            ExprAddress* Res = VisitToLValue(MemberAccess->Target);
            if (!Res) return nullptr;

            llvm::StringRef FieldName;
            if (auto Identifier = Cast<IdentifierNode>(MemberAccess->Member))
                FieldName = Identifier->Value;
            else
            {
                SendError(TypeErrorKind::MemberNotIdentifier, MemberAccess->Member);
                return nullptr;
            }

            QualType TargetType = Res->GetType().GetNotReferenceType();
            if (auto PtrType = TargetType.CastAs<PointerType>())
                TargetType = PtrType->GetBaseType();

            if (auto ClassTy = TargetType.CastAs<ClassType>())
            {
                MemberAccess->CompileTimeValue = ExprResult::CreateEmpty(ClassTy, MainArena);

                size_t ArgsCount = Call->Arguments.size();
                ArgsVector<QualType> ArgTypes;
                ArgTypes.reserve(ArgsCount);

                for (auto Arg : Call->Arguments)
                {
                    ExprResult* ArgValue = VisitToRValue(Arg);
                    if (!ArgValue)
                        return nullptr;

                    QualType ArgType = ArgValue->GetType();
                    if (!ArgType)
                        return nullptr;

                    ArgTypes.push_back(ArgType);
                }

                MethodTable::OverloadResult OverloadResult = ClassTy->FindBestMethodOverload(FieldName, ArgTypes);
                if (OverloadResult.Kind != MethodTable::OverloadResult::NotAvailable)
                {
                    const MethodOverload* Overload = OverloadResult.FirstOverload;
                    Call->ResolvedCallee = Overload->Callee;

                    for (size_t i = 0; i < ArgsCount; i++)
                        Call->Arguments[i]->ExpectedType = Overload->Args[i].GetType();

                    return ExprResult::CreateEmpty(Overload->Callee->FuncType->GetReturnType(), MainArena);
                }

                Array<std::string> ErrorContext = { ClassTy->GetName().str() + "." + FieldName.str() };
                ErrorContext.Reserve(ArgsCount);

                for (auto Arg : ArgTypes)
                    ErrorContext.Add(Arg->ToString());

                SendError(TypeErrorKind::NoFunctionOverload, Call->Callee, std::move(ErrorContext));
                return nullptr;
            }
        }

        return nullptr;
    }

    SemaResult *TypeChecker::VisitSubscript(SubscriptNode *Subscript)
    {
        ExprResult* TargetValue = VisitToRValue(Subscript->Target);
        ExprResult* IndexValue = VisitToRValue(Subscript->Index);

        if (!TargetValue || !IndexValue)
            return nullptr;

        DataType* TargetType = TargetValue->GetType().GetType();
        DataType* IndexType = IndexValue->GetType().GetType();

        DataType* Int64Type = CContext.GetIntegerType(64);
        DataType* UInt64Type = CContext.GetIntegerType(64, false);

        if (!IndexType->ImplicitCast(Int64Type) &&
            !IndexType->ImplicitCast(UInt64Type))
            return nullptr;


        if (auto ArrType = Cast<ArrayType>(TargetType))
        {
            Subscript->TargetType = ArrType;
            return MainArena.Create<ExprAddress>(ExprResult::CreateEmpty(ArrType->GetBaseType(), MainArena));
        }

        if (auto PtrType = Cast<PointerType>(TargetType))
        {
            Subscript->TargetType = PtrType;
            return MainArena.Create<ExprAddress>(ExprResult::CreateEmpty(PtrType->GetBaseType(), MainArena));
        }

        return nullptr;
    }

    SemaResult *TypeChecker::VisitExplicitCast(ExplicitCastNode *ECast)
    {
        QualType SrcType = VisitType(ECast->Type);
        ExprResult* Target = VisitToRValue(ECast->Target);

        if (!SrcType || !Target)
            return nullptr;

        if (ECast->IsBitCast)
        {
            if (Target->GetType()->GetSize() == SrcType->GetSize())
            {
                ECast->CompileTimeValue = ExprResult::CreateEmpty(SrcType, MainArena);
                return ECast->CompileTimeValue;
            }
        }
        else
        {
            if (auto CastedTarget = Target->ExplicitCast(SrcType, CContext))
            {
                ECast->CompileTimeValue = CastedTarget;
                return CastedTarget;
            }
        }

        SendError(TypeErrorKind::IncompatibleTypes, ECast->Line, ECast->Column,
            { SrcType->ToString(), Target->GetType()->ToString() });
        return nullptr;
    }

    SemaResult * TypeChecker::VisitSizeOf(SizeOfNode *SizeOf)
    {
        if (auto TypeNode = Cast<DataTypeNodeBase>(SizeOf->Target))
        {
            QualType Type = VisitType(TypeNode);
            if (!Type) return nullptr;
            SizeOf->CompileTimeValue = ExprResult::CreateInteger(
                CContext.GetIntegerType(32, false),
                Type->GetSize(), MainArena);
            return SizeOf->CompileTimeValue;
        }

        SemaResult* Value = VisitNode(SizeOf->Target);
        if (!Value) return nullptr;
        SizeOf->CompileTimeValue = ExprResult::CreateInteger(
            CContext.GetIntegerType(32, false),
            Value->GetType()->GetSize(), MainArena);
        return SizeOf->CompileTimeValue;
    }

    SemaResult * TypeChecker::VisitAlignOf(AlignOfNode *AlignOf)
    {
        if (auto TypeNode = Cast<DataTypeNodeBase>(AlignOf->Target))
        {
            QualType Type = VisitType(TypeNode);
            if (!Type) return nullptr;
            AlignOf->CompileTimeValue = ExprResult::CreateInteger(
                CContext.GetIntegerType(32, false),
                Type->GetAlignment(), MainArena);
            return AlignOf->CompileTimeValue;
        }

        SemaResult* Value = VisitNode(AlignOf->Target);
        if (!Value) return nullptr;
        AlignOf->CompileTimeValue = ExprResult::CreateInteger(
            CContext.GetIntegerType(32, false),
            Value->GetType()->GetAlignment(), MainArena);
        return AlignOf->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitConstruct(CallNode *Construct)
    {
        if (auto Identifier = Cast<IdentifierNode>(Construct->Callee))
        {
            QualType Type = CContext.GetClassType(Identifier->Value);
            auto ClassTy = Type.CastAs<ClassType>();
            if (!ClassTy)
            {
                // SendError
                return nullptr;
            }

            size_t ArgsCount = Construct->Arguments.size();
            ArgsVector<QualType> Arguments;
            Arguments.reserve(ArgsCount + 1);
            Arguments.emplace_back(CContext.GetPointerType(ClassTy));
            for (auto ArgNode : Construct->Arguments)
            {
                ExprResult* Arg = VisitToRValue(ArgNode);
                if (!Arg) return nullptr;

                Arguments.push_back(Arg->GetType());
            }

            FuncOverloadTable::OverloadResult OverloadResult = ClassTy->FindBestConstructorOverload(Arguments);
            if (OverloadResult.Kind != FuncOverloadTable::OverloadResult::NotAvailable)
            {
                const FunctionOverload* Overload = OverloadResult.FirstOverload;
                Construct->ResolvedCallee = Overload->Callee;

                for (size_t i = 0; i < ArgsCount; i++)
                    Construct->Arguments[i]->ExpectedType = Overload->Args[i + 1].GetType();
            }

            Construct->CompileTimeValue = ExprResult::CreateEmpty(Type, MainArena);
            return Construct->CompileTimeValue;
        }

        return nullptr;
    }

    SemaResult *TypeChecker::VisitVariable(VariableNode *Variable)
    {
        if (!InFunction)
        {
            DeclareGlobalVariable(Variable);
            return nullptr;
        }

        QualType VarType = VisitType(Variable->Type);
        if (!VarType) return nullptr;

        llvm::StringRef Name = Variable->Name;

        if (VarType->IsReferenceType())
        {
            ExprAddress* ValueAddr = VisitToLValue(Variable->Value);
            if (!ValueAddr) return nullptr;

            DeclareVariable(Name, ValueAddr, Variable);
            return nullptr;
        }

        ExprResult* Value = VisitToRValue(Variable->Value);
        if (!Value && VarType->IsClassType())
        {
            auto* ClassTy = StaticCast<ClassType>(VarType.GetType());
            FuncOverloadTable::OverloadResult OverloadResult =  ClassTy->FindBestConstructorOverload(
                { CContext.GetPointerType(ClassTy) });
            if (OverloadResult.Kind != FuncOverloadTable::OverloadResult::NotAvailable)
                Variable->ResolvedConstructor = OverloadResult.FirstOverload->Callee;
        }

        if (Value && !Value->GetType().ImplicitCast(VarType))
        {
            SendError(TypeErrorKind::AssignmentTypeMismatch,
                Variable,{ Name.str(),
                VarType->ToString(), Value->GetType()->ToString() });
            Value = ExprResult::CreateEmpty(VarType, MainArena);
            DeclareVariable(Name, MainArena.Create<ExprAddress>(Value), Variable);
            return nullptr;
        }

        if (!VarType.HasQualifier(QualType::CONST))
            Value = ExprResult::CreateEmpty(VarType, MainArena);

        DeclareVariable(Name, MainArena.Create<ExprAddress>(Value), Variable);
        return nullptr;
    }

    SemaResult *TypeChecker::VisitVariableConstruct(VariableConstructNode *Construct)
    {
        QualType VarType = VisitType(Construct->Type);
        if (!VarType) return nullptr;

        auto* ClassTy = VarType.CastAs<ClassType>();

        if (!ClassTy)
        {
            // SendError
            return nullptr;
        }

        ArgsVector<QualType> Args;
        Args.reserve(Construct->Arguments.size() + 1);
        Args.push_back(CContext.GetPointerType(VarType));

        for (auto* Arg : Construct->Arguments)
        {
            SemaResult* Res = VisitNode(Arg);
            if (!Res) continue;

            Args.push_back(Res->GetType());
        }

        FuncOverloadTable::OverloadResult OverloadResult = ClassTy->FindBestConstructorOverload(Args);
        if (OverloadResult.Kind != FuncOverloadTable::OverloadResult::NotAvailable)
        {
            const FunctionOverload* Overload = OverloadResult.FirstOverload;
            Construct->ResolvedCallee = Overload->Callee;

            for (size_t i = 0; i < Construct->Arguments.size(); i++)
                Construct->Arguments[i]->ExpectedType = Overload->Args[i + 1].GetType();
        }

        DeclareVariable(Construct->Name, MainArena.Create<ExprAddress>(
            ExprResult::CreateEmpty(VarType, MainArena)), Construct);

        return nullptr;
    }

    SemaResult *TypeChecker::VisitFunction(FunctionNode *Function)
    {
        ArgsVector<QualType> Params;
        CalleeBase* FuncCallee = CreateFunction(Function, Params);

        Functions.AddFunction(Function->Name, std::move(Params), StaticCast<FunctionCallee>(FuncCallee));
        FunctionBodies.Emplace(Function->Name, Function->Body,
            FuncCallee->FuncType->GetReturnType(), nullptr, Function->Params);
        return nullptr;
    }

    void TypeChecker::VisitMethod(FunctionNode *Method, ClassType* Type)
    {
        ArgsVector<QualType> Params;

        CalleeBase* FuncCallee = CreateFunction(Method, Params, Type);
        Type->AddMethod(Method->Name, std::move(Params), StaticCast<MethodCallee>(FuncCallee));
        FunctionBodies.Emplace(Method->Name, Method->Body, FuncCallee->FuncType->GetReturnType(),
            CContext.GetPointerType(Type), Method->Params);
    }

    void TypeChecker::VisitConstructor(ConstructorNode *Constructor, ClassType *Type)
    {
        QualType ThisType = CContext.GetPointerType(Type);

        ArgsVector<QualType> Params;

        Params.reserve(Constructor->Params.size() + 1);
        Params.push_back(ThisType);

        for (auto* Param : Constructor->Params)
        {
            QualType ParamType = VisitType(Param->Type);
            Params.push_back(ParamType);
        }

        QualType ReturnType = CContext.GetVoidType();

        auto ConstructorCallee = MainArena.Create<FunctionCallee>(CContext.GetFunctionType(ReturnType, Params));
        Constructor->ResolvedCallee = ConstructorCallee;

        Type->AddConstructor(std::move(Params), ConstructorCallee);
        FunctionBodies.Emplace(Type->GetName(), Constructor->Body,
            ReturnType, ThisType, Constructor->Params);
    }

    SemaResult *TypeChecker::VisitClass(ClassNode *Class)
    {
        ClassType* Type = CContext.CreateClassType(Class->Name);
        if (!Type)
        {
            // SendError
            return nullptr;
        }

        Classes.Emplace(Type, Class->Fields, Class->Methods, Class->Constructors);
        return nullptr;
    }

    SemaResult *TypeChecker::VisitMemberAccess(MemberAccessNode *MemberAccess)
    {
        ExprAddress* Res = VisitToLValue(MemberAccess->Target);
        if (!Res) return nullptr;

        llvm::StringRef FieldName;
        if (auto Identifier = Cast<IdentifierNode>(MemberAccess->Member))
            FieldName = Identifier->Value;
        else
        {
            SendError(TypeErrorKind::MemberNotIdentifier, MemberAccess->Member);
            return nullptr;
        }

        QualType TargetType = Res->GetType().GetNotReferenceType();
        if (auto PtrType = TargetType.CastAs<PointerType>())
            TargetType = PtrType->GetBaseType();

        if (auto ClassTy = TargetType.CastAs<ClassType>())
        {
            size_t Index = ClassTy->GetFieldIndex(FieldName);
            if (Index == ClassTy->GetFieldsCount())
            {
                SendError(TypeErrorKind::UndefinedVariable, MemberAccess, { FieldName.str() });
                return nullptr;
            }

            QualType FieldType = ClassTy->GetField(Index).Type;
            MemberAccess->ResolvedMemberIndex = Index;
            MemberAccess->CompileTimeValue = MainArena.Create<ExprAddress>(
                ExprResult::CreateEmpty(FieldType, MainArena));
            return MemberAccess->CompileTimeValue;
        }

        SendError(TypeErrorKind::AccessToNonClassType, MemberAccess);
        return nullptr;
    }

    SemaResult *TypeChecker::VisitIf(IfNode *If)
    {
        ExprResult* Cond = VisitToRValue(If->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->GetType().GetType();
        if (!CondType)
            return nullptr;

        if (!CondType->ImplicitCast(CContext.GetBoolType()))
        {
            SendError(TypeErrorKind::ConditionNotBool, If->Condition);
            return nullptr;
        }

        VisitNode(If->Branch);

        if (If->ElseBranch)
            VisitNode(If->ElseBranch);

        return nullptr;
    }

    SemaResult *TypeChecker::VisitWhile(WhileNode *While)
    {
        ExprResult* Cond = VisitToRValue(While->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->GetType().GetType();
        if (!CondType)
            return nullptr;

        if (!CondType->ImplicitCast(CContext.GetBoolType()))
        {
            SendError(TypeErrorKind::ConditionNotBool, While->Condition);
            return nullptr;
        }

        VisitNode(While->Branch);
        return nullptr;
    }

    SemaResult *TypeChecker::VisitFor(ForNode *For)
    {
        Variables.EnterScope();

        VisitNode(For->Initialization);

        ExprResult* Cond = VisitToRValue(For->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->GetType().GetType();
        if (!CondType)
            return nullptr;

        if (!CondType->ImplicitCast(CContext.GetBoolType()))
        {
            SendError(TypeErrorKind::ConditionNotBool, For->Condition);
            return nullptr;
        }
        VisitNode(For->Iteration);
        VisitNode(For->Body);

        Variables.ExitScope();

        return nullptr;
    }

    SemaResult *TypeChecker::VisitReturn(ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            if (FunctionReturnType.GetType() == CContext.GetVoidType())
            {
                SendError(TypeErrorKind::VoidReturnValue, Return->ReturnValue);
                return nullptr;
            }

            SemaResult* ReturnValue = VisitNode(Return->ReturnValue);
            if (!ReturnValue) return nullptr;

            if (!ReturnValue->GetType().ImplicitCast(FunctionReturnType))
                SendError(TypeErrorKind::ReturnTypeMismatch, Return->ReturnValue);

            return nullptr;
        }

        if (FunctionReturnType.GetType() != CContext.GetVoidType())
            SendError(TypeErrorKind::NonVoidMissingReturn, Return);

        return nullptr;
    }

    QualType TypeChecker::VisitType(DataTypeNodeBase *Type)
    {
        switch (Type->GetNodeKind())
        {
            case NodeKind::PrimitiveTypeNode:
            {
                auto Primitive = StaticCast<PrimitiveTypeNode>(Type);
                Primitive->ResolvedType = Primitive->Type;
                return { Primitive->Type, 0 };
            }
            case NodeKind::PointerTypeNode:
            {
                auto Ptr = StaticCast<PointerTypeNode>(Type);
                Ptr->ResolvedType = CContext.GetPointerType(VisitType(Ptr->BaseType));
                return { Ptr->ResolvedType, 0 };
            }
            case NodeKind::ReferenceTypeNode:
            {
                auto Ref = StaticCast<ReferenceTypeNode>(Type);
                Ref->ResolvedType = CContext.GetReferenceType(VisitType(Ref->BaseType));
                return { Ref->ResolvedType, 0 };
            }
            case NodeKind::ArrayTypeNode:
            {
                auto Arr = StaticCast<ArrayTypeNode>(Type);
                ExprResult* Length = VisitToRValue(Arr->Length);
                if (Length && Length->GetType()->IsIntegerType())
                {
                    UInt64 Len = Length->GetType()->IsSignedIntegerType() ?
                        static_cast<UInt64>(Length->GetInt()) : Length->GetUInt();
                    Arr->ResolvedType = CContext.GetArrayType(VisitType(Arr->BaseType), Len);
                    return { Arr->ResolvedType, 0 };
                }

                // SendError
                return {};
            }
            case NodeKind::ClassTypeNode:
            {
                auto Class = StaticCast<ClassTypeNode>(Type);
                auto ClassTy = CContext.GetClassType(Class->Name);
                if (!ClassTy)
                {
                    // SendError
                    return {};
                }

                Class->ResolvedType = ClassTy;
                return { ClassTy, 0 };
            }
            case NodeKind::QualTypeNode:
            {
                auto QualTy = StaticCast<QualTypeNode>(Type);
                QualType QType = VisitType(QualTy->Type);
                QualTy->ResolvedType = QType.GetType();
                QType.AddQualifiers(QualTy->Quals);
                return QType;
            }
            case NodeKind::TypeOfNode:
            {
                auto TypeOf = StaticCast<TypeOfNode>(Type);
                SemaResult* Value = VisitNode(TypeOf->Target);
                if (!Value) return nullptr;
                TypeOf->ResolvedType = Value->GetType().GetType();
                return Value->GetType();
            }
            default:
                VoltUnreachableFmt("Cannot resolve type node: {}", Type->GetName());
        }
    }

    ExprResult* TypeChecker::GetRValue(SemaResult *Value)
    {
        if (auto Res = Cast<ExprResult>(Value))
            return Res;

        if (auto Addr = Cast<ExprAddress>(Value))
            return Addr->GetValue();

        VoltUnreachable("Invalid SemaResult");
    }

    ExprAddress* TypeChecker::VisitToLValue(ASTNode *Node)
    {
        SemaResult* Result = VisitNode(Node);
        if (!Result) return nullptr;
        auto Addr = Cast<ExprAddress>(Result);
        if (!Addr)
        {
            SendError(TypeErrorKind::NonLValue, Node);
            return nullptr;
        }
        return Addr;
    }

    ExprAddress* TypeChecker::VisitToLValueAndCheckConst(ASTNode *Node)
    {
        ExprAddress* Addr = VisitToLValue(Node);
        if (!Addr) return nullptr;

        if (Addr->GetType().HasQualifier(QualType::CONST))
        {
            SendError(TypeErrorKind::AssignReadOnlyType, Node);
            return nullptr;
        }

        return Addr;
    }

    CalleeBase* TypeChecker::CreateFunction(FunctionNode *Function,
        ArgsVector<QualType>& Params, ClassType* Owner)
    {
        Params.reserve(Function->Params.size());

        for (auto* Param : Function->Params)
        {
            QualType ParamType = VisitType(Param->Type);
            Params.push_back(ParamType);
        }

        QualType ReturnType = VisitType(Function->ReturnType);
        Function->ResolvedCallee = Owner ? MainArena.Create<MethodCallee>(CContext.GetMethodType(ReturnType, Params,
                                  CContext.GetPointerType(Owner)), Owner) :
                                           MainArena.Create<FunctionCallee>(
                                           CContext.GetFunctionType(ReturnType, Params));
        return Function->ResolvedCallee;
    }

    bool TypeChecker::ImplicitCastOrError(DataType *&Src, DataType* Dst, size_t Line, size_t Column)
    {
        if (Src->ImplicitCast(Dst))
            return true;

        SendError(TypeErrorKind::IncompatibleTypes, Line, Column,
            { Src->ToString(), Dst->ToString() });
        return false;
    }

    void TypeChecker::DeclareGlobalVariable(VariableNode *Variable)
    {
        QualType VarType = VisitType(Variable->Type);
        if (!VarType) return;

        ExprResult* Value = VisitToRValue(Variable->Value);

        if (Value && !Value->GetType().ImplicitCast(VarType))
        {
            SendError(TypeErrorKind::AssignmentTypeMismatch,
                Variable,{ Variable->Name.str(),
                VarType->ToString(), Value->GetType()->ToString() });

            Value = ExprResult::CreateEmpty(VarType, MainArena);
            GlobalVariables[Variable->Name] = MainArena.Create<ExprAddress>(Value);
            return;
        }

        if (!VarType.HasQualifier(QualType::CONST))
            Value = ExprResult::CreateEmpty(VarType, MainArena);

        GlobalVariables[Variable->Name] = MainArena.Create<ExprAddress>(Value);
    }

    void TypeChecker::DeclareVariable(llvm::StringRef Name, ExprAddress* Addr, ASTNode* VarNode)
    {
        VariableStack::VariableDeclKind Kind = Variables.DeclareVariable(Name, Addr);
        if (Kind == VariableStack::AlreadyExists)
            SendError(TypeErrorKind::DoubleVariableDeclaration, VarNode, { Name.str() });
    }

    void TypeChecker::DeclareAndAddParams(llvm::ArrayRef<ParamNode *> ParamNodes, ArgsVector<QualType> &ParamTypes)
    {
        for (auto* Param : ParamNodes)
        {
            QualType ParamType = VisitType(Param->Type);
            ParamTypes.push_back(ParamType);
            Variables.DeclareVariable(Param->Name, MainArena.Create<ExprAddress>(
                            ExprResult::CreateEmpty(ParamType, MainArena)));
        }
    }

    void TypeChecker::VisitClassAndMethodDecls()
    {
        for (const ClassData& Data : Classes)
        {
            ClassType* ClassTy = Data.ClassTy;

            size_t FieldIndex = 0;
            for (auto Field : Data.Fields)
            {
                ClassTy->AddField(Field->Name, VisitType(Field->Type));
                if (Field->IsImplemented)
                    ClassTy->ImplementField(FieldIndex);
                ++FieldIndex;
            }
            ClassTy->FinishInitializing();

            for (auto* Method : Data.Methods)
                VisitMethod(Method, ClassTy);

            for (auto* Constructor : Data.Constructors)
                VisitConstructor(Constructor, ClassTy);
        }
    }

    void TypeChecker::VisitFunctionBodies()
    {
        for (const auto& Data : FunctionBodies)
        {
            Variables.EnterScope();

            ArgsVector<QualType> Params;
            if (Data.ThisType)
            {
                Params.reserve(Data.Params.size());
                Variables.DeclareVariable("this", MainArena.Create<ExprAddress>(
                                ExprResult::CreateEmpty(Data.ThisType, MainArena)));
            }
            else
                Params.reserve(Data.Params.size());

            for (auto* Param : Data.Params)
            {
                QualType ParamType = Param->Type->ResolvedType;
                Params.push_back(ParamType);
                Variables.DeclareVariable(Param->Name, MainArena.Create<ExprAddress>(
                                ExprResult::CreateEmpty(ParamType, MainArena)));
            }

            FunctionReturnType = Data.ReturnType;
            InFunction = true;
            VisitBlock(Cast<BlockNode>(Data.Body));
            FunctionReturnType = {};
            InFunction = false;

            Variables.ExitScope();
        }
    }
}
