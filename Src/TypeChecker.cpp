//
// Created by bohdan on 15.01.26.
//

#include "Volt/Core/TypeChecker/TypeChecker.h"

namespace Volt
{
    void TypeChecker::WriteErrors(std::ostream& Os) const
    {
        for (const TypeError& Error : Errors)
            Os << "TypeError: " << Error.ToString() <<
                " At position: [" << Error.Line << ":" << Error.Column << "]\n";
    }

    ExprResult *TypeChecker::VisitNode(ASTNode *Node)
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
        if (auto Binary = Cast<BinaryOpNode>(Node))
            return VisitBinary(Binary);
        if (auto Call = Cast<CallNode>(Node))
            return VisitCall(Call);
        if (auto Subscript = Cast<SubscriptNode>(Node))
            return VisitSubscript(Subscript);
        if (auto ECast = Cast<ExplicitCastNode>(Node))
            return VisitExplicitCast(ECast);
        if (auto Variable = Cast<VariableNode>(Node))
            return VisitVariable(Variable);
        if (auto Function = Cast<FunctionNode>(Node))
            return VisitFunction(Function);
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

        if (!FunctionParams.empty())
        {
            for (const auto& [Name, Type] : FunctionParams)
                DeclareVariable(Name, Type);
        }

        for (auto Statement : Block->Statements)
            VisitNode(Statement);

        ExitScope();
    }

    ExprResult *TypeChecker::VisitInt(IntegerNode *Int)
    {
        int BitWidth = 0;
        switch (Int->Type)
        {
            case IntegerNode::BYTE:
                BitWidth = 8;
                break;
            case IntegerNode::INT:
                BitWidth = 32;
                break;
            case IntegerNode::LONG:
                BitWidth = 64;
                break;
            default:
                return nullptr;
        }

        Int->CompileTimeValue = ExprResult::CreateInteger(
            QualType(CContext.GetIntegerType(BitWidth), QualType::CONST), Int->Value, MainArena);
        return Int->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitFloat(FloatingPointNode *Float)
    {
        int BitWidth = 0;
        switch (Float->Type)
        {
            case FloatingPointNode::FLOAT:
                BitWidth = 32;
                break;
            case FloatingPointNode::DOUBLE:
                BitWidth = 64;
                break;
            default:
                return nullptr;
        }

        Float->CompileTimeValue = ExprResult::CreateFloat(
            QualType(CContext.GetFPType(BitWidth), QualType::CONST), Float->Value, MainArena);
        return Float->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitBool(BoolNode *Bool)
    {
        Bool->CompileTimeValue = ExprResult::CreateBool(
            QualType(CContext.GetBoolType(), QualType::CONST), Bool->Value, MainArena);
        return Bool->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitChar(CharNode *Char)
    {
        Char->CompileTimeValue = ExprResult::CreateChar(
            QualType(CContext.GetCharType(), QualType::CONST), Char->Value, MainArena);
        return Char->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitString(StringNode *String)
    {
        String->CompileTimeValue = ExprResult::CreateEmpty(
            QualType(CContext.GetPointerType(QualType(CContext.GetCharType(),
                QualType::CONST)), QualType::CONST), MainArena);
        return String->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitArray(ArrayNode *Array)
    {
        llvm::ArrayRef<ASTNode*> Elements = Array->Elements;

        if (Elements.empty())
            return nullptr;

        QualType ElementsType;
        bool HasErrors = false;
        for (auto El : Elements)
        {
            ExprResult* ElValue = VisitNode(El);
            if (!ElValue)
                return nullptr;

            QualType ElType = ElValue->Type;
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

    ExprResult *TypeChecker::VisitIdentifier(IdentifierNode *Identifier)
    {
        QualType VarType = GetVariable(Identifier->Value.str());
        if (!VarType)
        {
            SendError(TypeErrorKind::UndefinedVariable, Identifier, { Identifier->Value.str() });
            return nullptr;
        }

        Identifier->CompileTimeValue = ExprResult::CreateEmpty(VarType, MainArena);
        return Identifier->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitRef(RefNode *Ref)
    {
        ExprResult* RefValue = GetLValue(Ref->Target, true);
        if (!RefValue)
            return nullptr;

        QualType RefType = RefValue->Type;
        if (!RefType)
            return nullptr;

        Ref->CompileTimeValue = ExprResult::CreateEmpty(
            QualType(CContext.GetPointerType(RefType), 0), MainArena); // <-
        return Ref->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitUnref(UnrefNode *Unref)
    {
        ExprResult* UnrefValue = VisitNode(Unref->Target);
        if (!UnrefValue)
            return nullptr;

        QualType Type = UnrefValue->Type;
        if (!Type)
            return nullptr;

        if (auto PtrType = Cast<PointerType>(Type.GetType()))
        {
            Unref->CompileTimeValue = ExprResult::CreateEmpty(PtrType->BaseType, MainArena);
            return Unref->CompileTimeValue;
        }

        return nullptr;
    }

    ExprResult *TypeChecker::VisitSuffix(SuffixOpNode *Suffix)
    {
        ExprResult* SuffixValue = VisitNode(Suffix->Operand);
        if (!SuffixValue)
            return nullptr;

        QualType SuffixType = SuffixValue->Type;
        if (!SuffixType)
            return nullptr;

        switch (Suffix->Type)
        {
            case OperatorType::INC:
            case OperatorType::DEC:
            {
                if (SuffixType->IsIntegerType())
                {
                    Suffix->CompileTimeValue = ExprResult::CreateEmpty(SuffixType, MainArena);
                    return Suffix->CompileTimeValue;
                }

                SendError(TypeErrorKind::InvalidUnaryOperator, Suffix,
                    { Operator::ToString(Suffix->Type), SuffixType->ToString() });
                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    ExprResult *TypeChecker::VisitPrefix(PrefixOpNode *Prefix)
    {
        ExprResult* PrefixValue = GetLValue(Prefix->Operand);
        if (!PrefixValue)
            return nullptr;

        QualType PrefixType = PrefixValue->Type;
        if (!PrefixType)
            return nullptr;

        TypeCategory Category = PrefixType->GetCategory();

        switch (Prefix->Type)
        {
            case OperatorType::INC:
            case OperatorType::DEC:
            {
                if (Category == TypeCategory::INTEGER || Category == TypeCategory::CHAR || Category == TypeCategory::FLOATING_POINT)
                {
                    Prefix->CompileTimeValue = ExprResult::CreateEmpty(PrefixType,  MainArena);
                    return Prefix->CompileTimeValue;
                }
                SendError(TypeErrorKind::InvalidUnaryOperator, Prefix,
                    { Operator::ToString(Prefix->Type), PrefixType->ToString() });

                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    ExprResult *TypeChecker::VisitUnary(UnaryOpNode *Unary)
    {
        ExprResult* Operand = VisitNode(Unary->Operand);
        if (!Operand)
            return nullptr;

        if (auto Value = ExprResult::ResolveUnary(Operand, Unary->Type, CContext))
        {
            Unary->CompileTimeValue = Value;
            return Value;
        }

        return nullptr;
    }

    ExprResult *TypeChecker::VisitBinary(BinaryOpNode *Binary)
    {
        ExprResult* Left = nullptr;
        if (Cast<AssignmentNode>(Binary))
            Left = GetLValue(Binary->Left);
        else
            Left = VisitNode(Binary->Left);

        ExprResult* Right = VisitNode(Binary->Right);

        if (!Left || !Right)
            return nullptr;

        Binary->CompileTimeValue = ExprResult::ResolveBinary(Left, Right, Binary->Type, CContext);
        Binary->LeftOperandType = Left->Type.GetType();
        Binary->RightOperandType = Right->Type.GetType();
        return Binary->CompileTimeValue;
    }

    ExprResult *TypeChecker::VisitCall(CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            const std::string& Name = Identifier->Value.str();
            size_t ArgsCount = Call->Arguments.size();
            SmallVec8<QualType> ArgTypes;
            ArgTypes.reserve(ArgsCount);

            for (auto Arg : Call->Arguments)
            {
                ExprResult* ArgValue = VisitNode(Arg);
                if (!ArgValue)
                    return nullptr;

                QualType ArgType = ArgValue->Type;
                if (!ArgType)
                    return nullptr;

                ArgTypes.push_back(ArgType);
            }

            FunctionSignature Signature(Name, ArgTypes);

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

        if (auto Type = Cast<DataTypeNodeBase>(Call->Callee))
        {
            if (Call->Arguments.size() != 1)
                return nullptr;

            VisitNode(Call->Arguments[0]);
            return ExprResult::CreateEmpty(VisitType(Type), MainArena);
        }

        SendError(TypeErrorKind::InvalidCalleeType, Call->Callee);
        return nullptr;
    }

    ExprResult *TypeChecker::VisitSubscript(SubscriptNode *Subscript)
    {
        ExprResult* TargetValue = VisitNode(Subscript->Target);
        ExprResult* IndexValue = VisitNode(Subscript->Index);

        if (!TargetValue || !IndexValue)
            return nullptr;

        DataType* TargetType = TargetValue->Type.GetType();
        DataType* IndexType = IndexValue->Type.GetType();

        DataType* Int32Type = CContext.GetIntegerType(32);

        if (!ImplicitCastOrError(IndexType, Int32Type, Subscript->Index->Line, Subscript->Index->Column))
            return nullptr;


        if (auto ArrType = Cast<ArrayType>(TargetType))
        {
            Subscript->TargetType = ArrType;
            return ExprResult::CreateEmpty(ArrType->BaseType, MainArena);
        }

        if (auto PtrType = Cast<PointerType>(TargetType))
        {
            Subscript->TargetType = PtrType;
            return ExprResult::CreateEmpty(PtrType->BaseType, MainArena);
        }

        return nullptr;
    }

    ExprResult *TypeChecker::VisitExplicitCast(ExplicitCastNode *ECast)
    {
        QualType SrcType = VisitType(ECast->Type);
        ExprResult* Target = VisitNode(ECast->Target);

        if (!SrcType || !Target)
            return nullptr;

        if (Target->ExplicitCast(SrcType))
        {
            ECast->CompileTimeValue = Target;
            return Target;
        }

        SendError(TypeErrorKind::IncompatibleTypes, ECast->Line, ECast->Column,
            { SrcType->ToString(), Target->Type->ToString() });
        return nullptr;
    }

    ExprResult *TypeChecker::VisitVariable(VariableNode *Variable)
    {
        QualType VarType = VisitType(Variable->Type);

        if (!VarType)
            return nullptr;

        if (auto RefType = VarType.CastAs<ReferenceType>())
        {
            ExprResult* Value = GetLValue(Variable->Value);

            if (!Value || !RefType->CanBind(Value->Type))
            {
                SendError(TypeErrorKind::InvalidBind, Variable);
                return nullptr;
            }

            DeclareVariable(Variable->Name.str(), VarType);
            return nullptr;
        }

        ExprResult* Value = VisitNode(Variable->Value);

        if (Value && !Value->Type.ImplicitCast(VarType))
        {
            SendError(TypeErrorKind::AssignmentTypeMismatch,
                Variable,{ Variable->Name.str(),
                VarType->ToString(), Value->Type->ToString() });
            return nullptr;
        }

        DeclareVariable(Variable->Name.str(), VarType);
        return nullptr;
    }

    ExprResult *TypeChecker::VisitFunction(FunctionNode *Function)
    {
        SmallVec8<QualType> Params;
        Params.reserve(Function->Params.size());
        FunctionParams.reserve(Function->Params.size());
        for (const auto& Param : Function->Params)
        {
            QualType ParamType = VisitType(Param->Type);
            Params.push_back(ParamType); // <-
            FunctionParams.emplace_back(Param->Name.str(), ParamType);
        }

        FunctionSignature Signature(Function->Name.str(), Params);
        QualType ReturnType = VisitType(Function->ReturnType);

        auto FuncCallee = MainArena.Create<FunctionCallee>(ReturnType, nullptr);
        Function->ResolvedCallee = FuncCallee;
        Functions[Signature] = FuncCallee;

        FunctionReturnType = ReturnType;
        VisitBlock(Cast<BlockNode>(Function->Body));
        FunctionReturnType = QualType();

        return nullptr;
    }

    ExprResult *TypeChecker::VisitIf(IfNode *If)
    {
        ExprResult* Cond = VisitNode(If->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->Type.GetType();
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

    ExprResult *TypeChecker::VisitWhile(WhileNode *While)
    {
        ExprResult* Cond = VisitNode(While->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->Type.GetType();
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

    ExprResult *TypeChecker::VisitFor(ForNode *For)
    {
        VisitNode(For->Initialization);

        ExprResult* Cond = VisitNode(For->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->Type.GetType();
        if (!CondType)
            return nullptr;

        if (!CondType->ImplicitCast(CContext.GetBoolType()))
        {
            SendError(TypeErrorKind::ConditionNotBool, For->Condition);
            return nullptr;
        }
        VisitNode(For->Iteration);
        VisitNode(For->Body);

        return nullptr;
    }

    ExprResult *TypeChecker::VisitReturn(ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            if (FunctionReturnType.GetType() == CContext.GetVoidType())
            {
                SendError(TypeErrorKind::VoidReturnValue, Return->ReturnValue);
                return nullptr;
            }

            QualType ReturnType = VisitNode(Return->ReturnValue)->Type;
            if (!ReturnType.ImplicitCast(FunctionReturnType))
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
            ExprResult* Length = VisitNode(Array->Length);
            if (Length && Length->Type->IsIntegerType())
            {
                Array->ResolvedType = CContext.GetArrayType(VisitType(Array->BaseType), Length->Int);
                return { Array->ResolvedType, 0 };
            }

            throw std::runtime_error("Array length mast be defined in compiler time");
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

    ExprResult *TypeChecker::GetLValue(ASTNode *Node, bool IgnoreConstants)
    {
        ExprResult* Value = nullptr;

        if (auto Identifier = Cast<IdentifierNode>(Node))
            Value = VisitIdentifier(Identifier);
        else if (auto Subscript = Cast<SubscriptNode>(Node))
            Value = VisitSubscript(Subscript);
        else if (auto Unref = Cast<UnrefNode>(Node))
            Value = VisitUnref(Unref);
        else
        {
            SendError(TypeErrorKind::AssignNonLValue, Node->Line, Node->Column);
            return nullptr;
        }

        if (!Value)
            return nullptr;

        if (!IgnoreConstants)
        {
            QualType Type = GetNotReferenceType(Value->Type);
            if (Type.HasQualifier(QualType::CONST))
            {
                SendError(TypeErrorKind::AssignReadOnlyType, Node->Line, Node->Column);
                return nullptr;
            }
        }

        return Value;
    }

    QualType TypeChecker::GetNotReferenceType(QualType Type)
    {
        if (auto RefType = Type.CastAs<ReferenceType>())
            Type = RefType->BaseType;

        return Type;
    }

    bool TypeChecker::CanCastPointers(PointerType *Src, PointerType *Dst)
    {
        return Src == Dst || Dst->BaseType->GetCategory() == TypeCategory::VOID;
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
        for (const CTimeScopeEntry& Entry : ScopeStack.Back())
        {
            if (Entry.Previous)
                Variables[Entry.Name] = Entry.Previous;
            else
                Variables.erase(Entry.Name);
        }

        ScopeStack.Pop();
    }

    void TypeChecker::DeclareVariable(const std::string &Name, QualType Type)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            ScopeStack.Back().Emplace(Name, Iter->second);

        Variables[Name] = ExprResult::CreateEmpty(Type,  MainArena);
        ScopeStack.Back().Add({ Name, nullptr });
    }

    QualType TypeChecker::GetVariable(const std::string &Name)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            return Iter->second->Type;

        return { };
    }
}
