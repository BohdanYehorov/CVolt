//
// Created by bohdan on 15.01.26.
//

#include "Volt/Core/TypeChecker/TypeChecker.h"

#include "Volt/Core/TypeChecker/ExprAddress.h"
#include "Volt/Support/ErrorHandling.h"

namespace Volt
{
    SemaResult* TypeChecker::VisitNode(ASTNode *Node)
    {
        if (auto Sequence = Cast<SequenceNode>(Node))
        {
            VisitSequence(Sequence);
            return nullptr;
        }

        if (auto Block = Cast<BlockNode>(Node))
        {
            VisitBlock(Block);
            return nullptr;
        }

        if (auto Int = Cast<IntegerNode>(Node))
            return VisitInt(Int);
        if (auto Float = Cast<FloatingPointNode>(Node))
            return VisitFloat(Float);
        if (auto Bool = Cast<BoolNode>(Node))
            return VisitBool(Bool);
        if (auto Char = Cast<CharNode>(Node))
            return VisitChar(Char);
        if (auto String = Cast<StringNode>(Node))
            return VisitString(String);
        if (auto Array = Cast<ArrayNode>(Node))
            return VisitArray(Array);
        if (auto NullPtr = Cast<NullPointerNode>(Node))
            return VisitNullPointer(NullPtr);
        if (auto Identifier = Cast<IdentifierNode>(Node))
            return VisitIdentifier(Identifier);
        if (auto Ref = Cast<RefNode>(Node))
            return VisitRef(Ref);
        if (auto Unref = Cast<UnrefNode>(Node))
            return VisitUnref(Unref);
        if (auto Suffix = Cast<SuffixOpNode>(Node))
            return VisitSuffix(Suffix);
        if (auto Prefix = Cast<PrefixOpNode>(Node))
            return VisitPrefix(Prefix);
        if (auto Unary = Cast<UnaryOpNode>(Node))
            return VisitUnary(Unary);
        if (auto Assignment = Cast<AssignmentNode>(Node))
            return VisitAssignment(Assignment);
        if (auto Comparison = Cast<ComparisonNode>(Node))
            return VisitComparison(Comparison);
        if (auto Binary = Cast<BinaryOpNode>(Node))
            return VisitBinary(Binary);
        if (auto Call = Cast<CallNode>(Node))
            return VisitCall(Call);
        if (auto Subscript = Cast<SubscriptNode>(Node))
            return VisitSubscript(Subscript);
        if (auto ECast = Cast<ExplicitCastNode>(Node))
            return VisitExplicitCast(ECast);
        if (auto Construct = Cast<ConstructNode>(Node))
            return VisitConstruct(Construct);
        if (auto Variable = Cast<VariableNode>(Node))
            return VisitVariable(Variable);
        if (auto Function = Cast<FunctionNode>(Node))
            return VisitFunction(Function);
        if (auto Class = Cast<ClassNode>(Node))
            return VisitClass(Class);
        if (auto MemberAccess = Cast<MemberAccessNode>(Node))
            return VisitMemberAccess(MemberAccess);
        if (auto If = Cast<IfNode>(Node))
            return VisitIf(If);
        if (auto While = Cast<WhileNode>(Node))
            return VisitWhile(While);
        if (auto For = Cast<ForNode>(Node))
            return VisitFor(For);
        if (auto Return = Cast<ReturnNode>(Node))
            return VisitReturn(Return);

        if (Cast<BreakNode>(Node)) return nullptr;
        if (Cast<ContinueNode>(Node)) return nullptr;

        return nullptr;
    }

    void TypeChecker::VisitSequence(SequenceNode *Sequence)
    {
        for (auto Statement : Sequence->Statements)
            VisitNode(Statement);
    }

    void TypeChecker::VisitBlock(BlockNode *Block)
    {
        EnterScope();
        for (auto Statement : Block->Statements)
            VisitNode(Statement);
        ExitScope();
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
        ExprAddress* VarAddr = GetVariable(Identifier->Value.str());
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
                    ExprResult::CreateEmpty(PtrType->BaseType, MainArena));
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
            case OperatorType::INC: AssignmentType = OperatorType::ADD_ASSIGN; break;
            case OperatorType::DEC: AssignmentType = OperatorType::SUB_ASSIGN; break;
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
            case OperatorType::INC: AssignmentType = OperatorType::ADD_ASSIGN; break;
            case OperatorType::DEC: AssignmentType = OperatorType::SUB_ASSIGN; break;
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
            case ADD: break;
            case SUB: Operand = Operand->CreateNeg(CContext); break;
            case BIT_NOT: Operand = Operand->CreateBitNot(CContext); break;
            case LOGICAL_NOT: Operand = Operand->CreateNot(CContext); break;
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
            case ADD:     Result = Left->CreateAdd(Right, CContext); break;
            case SUB:     Result = Left->CreateSub(Right, CContext); break;
            case MUL:     Result = Left->CreateMul(Right, CContext); break;
            case DIV:     Result = Left->CreateDiv(Right, CContext); break;
            case MOD:     Result = Left->CreateMod(Right, CContext); break;
            case BIT_AND: Result = Left->CreateBitAnd(Right, CContext); break;
            case BIT_OR:  Result = Left->CreateBitOr(Right, CContext); break;
            case BIT_XOR: Result = Left->CreateBitXor(Right, CContext); break;
            case RSHIFT:  Result = Left->CreateBitRShift(Right, CContext); break;
            case LSHIFT:  Result = Left->CreateBitLShift(Right, CContext); break;
            default: VoltUnreachable("Unknown binary operator");
        }

        Binary->CompileTimeValue = Result;
        return Result;
    }

    SemaResult *TypeChecker::VisitCall(CallNode *Call)
    {
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
            const std::string& Name = Identifier->Value.str();
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

            FunctionSignature Signature(Name, std::move(ArgTypes));

            if (auto Iter = TryGetOverload(Signature, Functions); Iter != Functions.end())
            {
                Call->ResolvedCallee = Iter->second;

                for (size_t i = 0; i < ArgsCount; i++)
                    Call->Arguments[i]->ExpectedType = Iter->first.Params[i].GetType();

                return ExprResult::CreateEmpty(Iter->second->ReturnType, MainArena);
            }

            if (auto Iter = TryGetOverload(Signature, BuiltinFuncTable.GetMap());
                Iter != BuiltinFuncTable.GetMap().end())
            {
                Call->ResolvedCallee = Iter->second;

                for (size_t i = 0; i < ArgsCount; i++)
                    Call->Arguments[i]->ExpectedType = Iter->first.Params[i].GetType();

                return ExprResult::CreateEmpty(Iter->second->ReturnType, MainArena);
            }

            Array<std::string> ErrorContext = { Name };
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

            std::string FieldName;
            if (auto Identifier = Cast<IdentifierNode>(MemberAccess->Member))
                FieldName = std::string(Identifier->Value);
            else
            {
                SendError(TypeErrorKind::MemberNotIdentifier, MemberAccess->Member);
                return nullptr;
            }

            QualType TargetType = Res->GetType().GetNotReferenceType();
            if (auto PtrType = TargetType.CastAs<PointerType>())
                TargetType = PtrType->BaseType;

            if (auto ClassTy = TargetType.CastAs<ClassType>())
            {
                MemberAccess->CompileTimeValue = ExprResult::CreateEmpty(ClassTy, MainArena);

                size_t ArgsCount = Call->Arguments.size();
                ArgsVector<QualType> ArgTypes;
                ArgTypes.reserve(ArgsCount + 1);
                ArgTypes.push_back(CContext.GetPointerType(ClassTy));

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

                FunctionSignature Signature(FieldName, std::move(ArgTypes));

                if (auto Iter = TryGetOverload(Signature, ClassTy->Methods); Iter != ClassTy->Methods.end())
                {
                    Call->ResolvedCallee = Iter->second;

                    for (size_t i = 0; i < ArgsCount; i++)
                        Call->Arguments[i]->ExpectedType = Iter->first.Params[i + 1].GetType();

                    return ExprResult::CreateEmpty(Iter->second->ReturnType, MainArena);
                }

                Array<std::string> ErrorContext = { ClassTy->Name + "." + FieldName };
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

        DataType* Int32Type = CContext.GetIntegerType(32);

        if (!ImplicitCastOrError(IndexType, Int32Type, Subscript->Index->Line, Subscript->Index->Column))
            return nullptr;


        if (auto ArrType = Cast<ArrayType>(TargetType))
        {
            Subscript->TargetType = ArrType;
            return MainArena.Create<ExprAddress>(ExprResult::CreateEmpty(ArrType->BaseType, MainArena));
        }

        if (auto PtrType = Cast<PointerType>(TargetType))
        {
            Subscript->TargetType = PtrType;
            return MainArena.Create<ExprAddress>(ExprResult::CreateEmpty(PtrType->BaseType, MainArena));
        }

        return nullptr;
    }

    SemaResult *TypeChecker::VisitExplicitCast(ExplicitCastNode *ECast)
    {
        QualType SrcType = VisitType(ECast->Type);
        ExprResult* Target = VisitToRValue(ECast->Target);

        if (!SrcType || !Target)
            return nullptr;

        if (auto CastedTarget = Target->ExplicitCast(SrcType, CContext))
        {
            ECast->CompileTimeValue = CastedTarget;
            return CastedTarget;
        }

        SendError(TypeErrorKind::IncompatibleTypes, ECast->Line, ECast->Column,
            { SrcType->ToString(), Target->GetType()->ToString() });
        return nullptr;
    }

    SemaResult *TypeChecker::VisitConstruct(ConstructNode *Construct)
    {
        QualType Type = VisitType(Construct->Type);
        auto ClassTy = Type.CastAs<ClassType>();
        if (!ClassTy) return nullptr;

        size_t ArgsCount = Construct->Args.size();
        ArgsVector<QualType> Arguments;
        Arguments.reserve(ArgsCount + 1);
        Arguments.emplace_back(CContext.GetPointerType(ClassTy));
        for (auto ArgNode : Construct->Args)
        {
            ExprResult* Arg = VisitToRValue(ArgNode);
            if (!Arg) return nullptr;

            Arguments.push_back(Arg->GetType());
        }

        FunctionSignature Signature("Construct", std::move(Arguments));
        if (auto Iter = TryGetOverload(Signature, ClassTy->Methods); Iter != ClassTy->Methods.end())
        {
            Construct->ResolvedCallee = Iter->second;

            for (size_t i = 0; i < ArgsCount; i++)
                Construct->Args[i]->ExpectedType = Iter->first.Params[i + 1].GetType();
        }

        Construct->CompileTimeValue = ExprResult::CreateEmpty(Type, MainArena);
        return Construct->CompileTimeValue;
    }

    SemaResult *TypeChecker::VisitVariable(VariableNode *Variable)
    {
        QualType VarType = VisitType(Variable->Type);

        if (!VarType)
            return nullptr;

        if (!InFunction)
        {
            ExprResult* Value = VisitToRValue(Variable->Value);

            if (Value && !Value->GetType().ImplicitCast(VarType))
            {
                SendError(TypeErrorKind::AssignmentTypeMismatch,
                    Variable,{ Variable->Name.str(),
                    VarType->ToString(), Value->GetType()->ToString() });
                return nullptr;
            }

            if (!VarType.HasQualifier(QualType::CONST))
                Value = ExprResult::CreateEmpty(VarType, MainArena);

            GlobalVariables[Variable->Name.str()] = MainArena.Create<ExprAddress>(Value);
            return nullptr;
        }

        const std::string& Name{ Variable->Name };
        if (auto Iter = std::find_if(
            ScopeStack.Back().Begin(), ScopeStack.Back().End(),
            [&Name](const ScopeEntry& Entry) -> bool
            {
                return Entry.Name == Name;
            });
            Iter != ScopeStack.Back().End())
        {
            SendError(TypeErrorKind::DoubleVariableDeclaration, Variable, { Name });
            return nullptr;
        }

        if (VarType->IsReferenceType())
        {
            ExprAddress* ValueAddr = VisitToLValue(Variable->Value);
            if (!ValueAddr) return nullptr;

            DeclareVariable(Variable->Name.str(), ValueAddr);
            return nullptr;
        }

        ExprResult* Value = VisitToRValue(Variable->Value);

        if (Value && !Value->GetType().ImplicitCast(VarType))
        {
            SendError(TypeErrorKind::AssignmentTypeMismatch,
                Variable,{ Variable->Name.str(),
                VarType->ToString(), Value->GetType()->ToString() });
            return nullptr;
        }

        if (!VarType.HasQualifier(QualType::CONST))
            Value = ExprResult::CreateEmpty(VarType, MainArena);

        DeclareVariable(Variable->Name.str(), MainArena.Create<ExprAddress>(Value));
        return nullptr;
    }

    SemaResult *TypeChecker::VisitFunction(FunctionNode *Function)
    {
        EnterScope();

        FunctionSignature Signature;
        FunctionCallee* FuncCallee = CreateFunction(Function, Signature);

        Functions[Signature] = FuncCallee;

        FunctionReturnType = FuncCallee->ReturnType;
        InFunction = true;
        VisitBlock(Cast<BlockNode>(Function->Body));
        FunctionReturnType = {};
        InFunction = false;

        ExitScope();
        return nullptr;
    }

    void TypeChecker::VisitMethod(FunctionNode *Method, ClassType* Type)
    {
        EnterScope();

        FunctionSignature Signature;
        FunctionCallee* FuncCallee = CreateFunction(Method, Signature, CContext.GetPointerType(Type));
        Type->AddMethod(Signature, FuncCallee);

        FunctionReturnType = FuncCallee->ReturnType;
        InFunction = true;
        VisitBlock(Cast<BlockNode>(Method->Body));
        FunctionReturnType = {};
        InFunction = false;

        ExitScope();
    }

    SemaResult *TypeChecker::VisitClass(ClassNode *Class)
    {
        Array<Field> Fields;
        Fields.Reserve(Class->Fields.size());
        for (auto Field : Class->Fields)
            Fields.Emplace(std::string(Field->Name), VisitType(Field->Type));

        ClassType* Type = CContext.CreateClassType(std::string(Class->Name), Fields);
        if (!Type)
        {
            // SendError
            return nullptr;
        }

        for (auto Method : Class->Methods)
            VisitMethod(Method, Type);

        return nullptr;
    }

    SemaResult *TypeChecker::VisitMemberAccess(MemberAccessNode *MemberAccess)
    {
        ExprAddress* Res = VisitToLValue(MemberAccess->Target);
        if (!Res) return nullptr;

        std::string FieldName;
        if (auto Identifier = Cast<IdentifierNode>(MemberAccess->Member))
            FieldName = std::string(Identifier->Value);
        else
        {
            SendError(TypeErrorKind::MemberNotIdentifier, MemberAccess->Member);
            return nullptr;
        }

        QualType TargetType = Res->GetType().GetNotReferenceType();
        if (auto PtrType = TargetType.CastAs<PointerType>())
            TargetType = PtrType->BaseType;

        if (auto ClassTy = TargetType.CastAs<ClassType>())
        {
            size_t Index = ClassTy->GetFieldIndex(FieldName);
            if (Index == ClassTy->Fields.Length())
            {
                SendError(TypeErrorKind::UndefinedVariable, MemberAccess, { FieldName });
                return nullptr;
            }

            QualType FieldType = ClassTy->Fields[Index].Type;
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
        EnterScope();

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

        ExitScope();

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
        if (auto Primitive = Cast<PrimitiveTypeNode>(Type))
        {
            Primitive->ResolvedType = Primitive->Type;
            return { Primitive->Type, 0 };
        }
        if (auto Ptr = Cast<PointerTypeNode>(Type))
        {
            Ptr->ResolvedType = CContext.GetPointerType(VisitType(Ptr->BaseType));
            return { Ptr->ResolvedType, 0 };
        }
        if (auto Ref = Cast<ReferenceTypeNode>(Type))
        {
            Ref->ResolvedType = CContext.GetReferenceType(VisitType(Ref->BaseType));
            return { Ref->ResolvedType, 0 };
        }
        if (auto Array = Cast<ArrayTypeNode>(Type))
        {
            ExprResult* Length = VisitToRValue(Array->Length);
            if (Length && Length->GetType()->IsIntegerType())
            {
                Array->ResolvedType = CContext.GetArrayType(VisitType(Array->BaseType), Length->GetInt());
                return { Array->ResolvedType, 0 };
            }

            VoltUnreachable("Array length mast be defined in compiler time");
        }
        if (auto Class = Cast<ClassTypeNode>(Type))
        {
            auto ClassTy = CContext.GetClassType(std::string(Class->Name));
            if (!ClassTy)
            {
                // SendError
                return nullptr;
            }

            Class->ResolvedType = ClassTy;
            return { ClassTy, 0 };
        }
        if (auto QualTy = Cast<QualTypeNode>(Type))
        {
            QualType QType = VisitType(QualTy->Type);
            QualTy->ResolvedType = QType.GetType();
            QType.AddQualifiers(QualTy->Quals);
            return QType;
        }

        return {};
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

    FunctionCallee* TypeChecker::CreateFunction(FunctionNode *Function, FunctionSignature &Signature,
        QualType ThisType)
    {
        Signature.Name = Function->Name;

        if (ThisType)
        {
            Signature.Params.reserve(Function->Params.size() + 1);
            Signature.Params.push_back(ThisType);
            DeclareVariable("this", MainArena.Create<ExprAddress>(
            ExprResult::CreateEmpty(ThisType, MainArena)));
        }
        else
            Signature.Params.reserve(Function->Params.size());

        for (const auto& Param : Function->Params)
        {
            QualType ParamType = VisitType(Param->Type);
            Signature.Params.push_back(ParamType);
            DeclareVariable(std::string(Param->Name), MainArena.Create<ExprAddress>(
                ExprResult::CreateEmpty(ParamType, MainArena)));
        }

        QualType ReturnType = VisitType(Function->ReturnType);

        auto FuncCallee = MainArena.Create<FunctionCallee>(ReturnType, nullptr);
        Function->ResolvedCallee = FuncCallee;

        return FuncCallee;
    }

    bool TypeChecker::ImplicitCastOrError(DataType *&Src, DataType* Dst, size_t Line, size_t Column)
    {
        if (Src->ImplicitCast(Dst))
            return true;

        SendError(TypeErrorKind::IncompatibleTypes, Line, Column,
            { Src->ToString(), Dst->ToString() });
        return false;
    }

    void TypeChecker::EnterScope()
    {
        ScopeStack.Emplace();
    }

    void TypeChecker::ExitScope()
    {
        for (const auto& Entry : ScopeStack.Back())
        {
            if (Entry.Prev)
                Variables[Entry.Name] = Entry.Prev;
            else
                Variables.erase(Entry.Name);
        }

        ScopeStack.Pop();
    }

    void TypeChecker::DeclareVariable(const std::string &Name, ExprAddress* Addr)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            ScopeStack.Back().Emplace(Name, Iter->second);
        else
            ScopeStack.Back().Emplace(Name, nullptr);

        Variables[Name] = Addr;
    }

    ExprAddress* TypeChecker::GetVariable(const std::string &Name)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            return Iter->second;

        return nullptr;
    }
}
