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

    CTimeValue *TypeChecker::VisitNode(ASTNode *Node)
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

    CTimeValue *TypeChecker::VisitInt(IntegerNode *Int)
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

        // Int->ResolvedType = CContext.GetIntegerType(BitWidth);
        Int->CompileTimeValue = CTimeValue::CreateInteger(CContext.GetIntegerType(BitWidth), Int->Value, MainArena);
        return Int->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitFloat(FloatingPointNode *Float)
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

        //Float->ResolvedType = CContext.GetFPType(BitWidth);
        Float->CompileTimeValue = CTimeValue::CreateFloat(CContext.GetFPType(BitWidth), Float->Value, MainArena);
        return Float->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitBool(BoolNode *Bool)
    {
        // Bool->ResolvedType = CContext.GetBoolType();
        Bool->CompileTimeValue = CTimeValue::CreateBool(CContext.GetBoolType(), Bool->Value, MainArena);
        return Bool->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitChar(CharNode *Char)
    {
        // Char->ResolvedType = CContext.GetCharType();
        Char->CompileTimeValue = CTimeValue::CreateChar(CContext.GetCharType(), Char->Value, MainArena);
        return Char->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitString(StringNode *String)
    {
        // String->ResolvedType = CContext.GetPointerType(CContext.GetCharType());
        String->CompileTimeValue = CTimeValue::CreateEmpty(CContext.GetPointerType(CContext.GetCharType()), MainArena);
        return String->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitArray(ArrayNode *Array)
    {
        llvm::ArrayRef<ASTNode*> Elements = Array->Elements;

        if (Elements.empty())
            return nullptr;

        DataType* ElementsType = nullptr;
        bool HasErrors = false;
        for (auto El : Elements)
        {
            CTimeValue* ElValue = VisitNode(El);
            if (!ElValue)
                return nullptr;

            DataType* ElType = ElValue->Type;
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

        // Array->ResolvedType = CContext.GetArrayType(ElementsType, Elements.size());
        return CTimeValue::CreateEmpty(CContext.GetArrayType(ElementsType, Elements.size()), MainArena);
    }

    CTimeValue *TypeChecker::VisitIdentifier(IdentifierNode *Identifier)
    {
        DataType* VarType = GetVariable(Identifier->Value.str());
        if (!VarType)
            SendError(TypeErrorKind::UndefinedVariable, Identifier, { Identifier->Value.str() });

        // Identifier->ResolvedType = VarType;
        return CTimeValue::CreateEmpty(VarType, MainArena);
    }

    CTimeValue *TypeChecker::VisitRef(RefNode *Ref)
    {
        CTimeValue* RefValue = VisitNode(Ref->Target);
        if (!RefValue)
            return nullptr;

        DataType* RefType = RefValue->Type;
        if (!RefType)
            return nullptr;

        return CTimeValue::CreateEmpty(CContext.GetPointerType(RefType), MainArena);
    }

    CTimeValue *TypeChecker::VisitUnref(UnrefNode *Unref)
    {
        CTimeValue* UnrefValue = VisitNode(Unref->Target);
        if (!UnrefValue)
            return nullptr;

        DataType* Type = UnrefValue->Type;
        if (!Type)
            return nullptr;

        if (auto PtrType = Cast<PointerType>(Type))
        {
            // Unref->ResolvedType = PtrType->BaseType;
            Unref->CompileTimeValue = CTimeValue::CreateEmpty(PtrType->BaseType, MainArena);
            return Unref->CompileTimeValue;
        }

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitSuffix(SuffixOpNode *Suffix)
    {
        CTimeValue* SuffixValue = VisitNode(Suffix->Operand);
        if (!SuffixValue)
            return nullptr;

        DataType* SuffixType = SuffixValue->Type;
        if (!SuffixType)
            return nullptr;

        switch (Suffix->Type)
        {
            case OperatorType::INC:
            case OperatorType::DEC:
            {
                if (SuffixType->GetCategory() == TypeCategory::INTEGER)
                {
                    // Suffix->ResolvedType = SuffixType;
                    return CTimeValue::CreateEmpty(SuffixType, MainArena);
                }

                SendError(TypeErrorKind::InvalidUnaryOperator, Suffix,
                    { Operator::ToString(Suffix->Type), SuffixType->ToString() });
                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    CTimeValue *TypeChecker::VisitPrefix(PrefixOpNode *Prefix)
    {
        CTimeValue* PrefixValue = VisitNode(Prefix->Operand);
        if (!PrefixValue)
            return nullptr;

        DataType* PrefixType = PrefixValue->Type;
        if (!PrefixType)
            return nullptr;

        switch (Prefix->Type)
        {
            case OperatorType::INC:
            case OperatorType::DEC:
            {
                if (PrefixType->GetCategory() == TypeCategory::INTEGER)
                {
                    // Prefix->ResolvedType = PrefixType;
                    return CTimeValue::CreateEmpty(PrefixType,  MainArena);
                }
                SendError(TypeErrorKind::InvalidUnaryOperator, Prefix,
                    { Operator::ToString(Prefix->Type), PrefixType->ToString() });

                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    CTimeValue *TypeChecker::VisitUnary(UnaryOpNode *Unary)
    {
        CTimeValue* Operand = VisitNode(Unary->Operand);
        if (!Operand)
            return nullptr;

        if (auto Value = CTimeValue::ResolveUnary(Operand, Unary->Type, CContext))
        {
            Unary->CompileTimeValue = Value;
            return Value;
        }

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitBinary(BinaryOpNode *Binary)
    {
        CTimeValue* Left = VisitNode(Binary->Left);
        CTimeValue* Right = VisitNode(Binary->Right);

        if (!Left || !Right)
            return nullptr;

        Binary->CompileTimeValue = CTimeValue::ResolveBinary(Left, Right, Binary->Type, CContext);
        Binary->LeftOperandType = Left->Type;
        Binary->RightOperandType = Right->Type;
        return Binary->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitCall(CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            const std::string& Name = Identifier->Value.str();
            size_t ArgsCount = Call->Arguments.size();
            SmallVec8<DataType*> ArgTypes;
            ArgTypes.reserve(ArgsCount);

            for (auto Arg : Call->Arguments)
            {
                CTimeValue* ArgValue = VisitNode(Arg);
                if (!ArgValue)
                    return nullptr;

                DataType* ArgType = ArgValue->Type;
                if (!ArgType)
                    return nullptr;

                ArgTypes.push_back(ArgType);
            }

            FunctionSignature Signature(Name, ArgTypes);

            if (auto Iter = TryGetOverload(Signature, Functions); Iter != Functions.end())
            {
                Call->ResolvedCallee = Iter->second;

                for (size_t i = 0; i < ArgsCount; i++)
                    Call->Arguments[i]->ExpectedType = Iter->first.Params[i];

                return CTimeValue::CreateEmpty(Iter->second->ReturnType, MainArena);
            }

            if (auto Iter = TryGetOverload(Signature, BuiltinFuncTable.GetMap());
                Iter != BuiltinFuncTable.GetMap().end())
            {
                Call->ResolvedCallee = Iter->second;

                for (size_t i = 0; i < ArgsCount; i++)
                    Call->Arguments[i]->ExpectedType = Iter->first.Params[i];

                return CTimeValue::CreateEmpty(Iter->second->ReturnType, MainArena);
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

            // Call->ResolvedType = VisitType(Type);
            VisitNode(Call->Arguments[0]);
            return CTimeValue::CreateEmpty(VisitType(Type), MainArena);
        }

        SendError(TypeErrorKind::InvalidCalleeType, Call->Callee);
        return nullptr;
    }

    CTimeValue *TypeChecker::VisitSubscript(SubscriptNode *Subscript)
    {
        CTimeValue* TargetValue = VisitNode(Subscript->Target);
        CTimeValue* IndexValue = VisitNode(Subscript->Index);

        if (!TargetValue || !IndexValue)
            return nullptr;

        DataType* TargetType = TargetValue->Type;
        DataType* IndexType = IndexValue->Type;

        DataType* Int32Type = CContext.GetIntegerType(32);

        if (!ImplicitCastOrError(IndexType, Int32Type, Subscript->Index->Line, Subscript->Index->Column))
            return nullptr;

        // Subscript->Index->ResolvedType = IndexType;

        if (auto ArrType = Cast<ArrayType>(TargetType))
        {
            Subscript->TargetType = ArrType;
            // Subscript->ResolvedType = ArrType->BaseType;
            return CTimeValue::CreateEmpty(ArrType->BaseType, MainArena);
        }

        if (auto PtrType = Cast<PointerType>(TargetType))
        {
            Subscript->TargetType = PtrType;
            // Subscript->ResolvedType = PtrType->BaseType;
            return CTimeValue::CreateEmpty(PtrType->BaseType, MainArena);
        }

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitExplicitCast(ExplicitCastNode *ECast)
    {
        DataType* SrcType = VisitType(ECast->Type);
        CTimeValue* Target = VisitNode(ECast->Target);

        if (!SrcType || !Target)
            return nullptr;

        if (ExplicitCast(Target, SrcType))
        {
            ECast->CompileTimeValue = Target;
            return Target;
        }

        SendError(TypeErrorKind::IncompatibleTypes, ECast->Line, ECast->Column,
            { SrcType->ToString(), Target->Type->ToString() });
        return nullptr;
    }

    CTimeValue *TypeChecker::VisitVariable(VariableNode *Variable)
    {
        DataType* VarType = VisitType(Variable->Type);
        CTimeValue* Value = VisitNode(Variable->Value);

        if (!VarType)
            return nullptr;

        if (Value && !Value->Type->ImplicitCast(VarType))
        {
            SendError(TypeErrorKind::AssignmentTypeMismatch,
                Variable,{ Variable->Name.str(),
                VarType->ToString(), Value->Type->ToString() });
            return nullptr;
        }

        DeclareVariable(Variable->Name.str(), VarType);
        return nullptr;
    }

    CTimeValue *TypeChecker::VisitFunction(FunctionNode *Function)
    {
        SmallVec8<DataType*> Params;
        Params.reserve(Function->Params.size());
        FunctionParams.reserve(Function->Params.size());
        for (const auto& Param : Function->Params)
        {
            DataType* ParamType = VisitType(Param->Type);
            Params.push_back(ParamType);
            FunctionParams.emplace_back(Param->Name.str(), ParamType);
        }

        FunctionSignature Signature(Function->Name.str(), Params);
        DataType* ReturnType = VisitType(Function->ReturnType);

        auto FuncCallee = MainArena.Create<FunctionCallee>(ReturnType, nullptr);
        Function->ResolvedCallee = FuncCallee;
        Functions[Signature] = FuncCallee;

        FunctionReturnType = ReturnType;
        VisitBlock(Cast<BlockNode>(Function->Body));
        FunctionReturnType = nullptr;

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitIf(IfNode *If)
    {
        CTimeValue* Cond = VisitNode(If->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->Type;
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

    CTimeValue *TypeChecker::VisitWhile(WhileNode *While)
    {
        CTimeValue* Cond = VisitNode(While->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->Type;
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

    CTimeValue *TypeChecker::VisitFor(ForNode *For)
    {
        VisitNode(For->Initialization);

        CTimeValue* Cond = VisitNode(For->Condition);
        if (!Cond)
            return nullptr;

        DataType* CondType = Cond->Type;
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

    CTimeValue *TypeChecker::VisitReturn(ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            if (FunctionReturnType == CContext.GetVoidType())
            {
                SendError(TypeErrorKind::VoidReturnValue, Return->ReturnValue);
                return nullptr;
            }

            DataType* ReturnType = VisitNode(Return->ReturnValue)->Type;
            if (!ReturnType->ImplicitCast(FunctionReturnType))
                SendError(TypeErrorKind::ReturnTypeMismatch, Return->ReturnValue);

            return nullptr;
        }

        if (FunctionReturnType != CContext.GetVoidType())
            SendError(TypeErrorKind::NonVoidMissingReturn, Return);

        return nullptr;
    }

    DataType* TypeChecker::VisitType(DataTypeNodeBase *Type)
    {
        if (auto Primitive = Cast<PrimitiveTypeNode>(Type))
        {
            Primitive->ResolvedType = Primitive->Type;
            return Primitive->Type;
        }
        if (auto Ptr = Cast<PointerTypeNode>(Type))
        {
            Ptr->ResolvedType = CContext.GetPointerType(VisitType(Ptr->BaseType));
            return CContext.GetPointerType(VisitType(Ptr->BaseType));
        }
        if (auto Array = Cast<ArrayTypeNode>(Type))
        {
            CTimeValue* Length = VisitNode(Array->Length);
            if (Length && Length->Type->GetCategory() == TypeCategory::INTEGER)
            {
                Array->ResolvedType = CContext.GetArrayType(VisitType(Array->BaseType), Length->Int);
                return CContext.GetArrayType(VisitType(Array->BaseType), Length->Int);
            }

            throw std::runtime_error("Array length mast be defined in compiler time");
        }

        return nullptr;
    }

    CTimeValue *TypeChecker::GetLValue(ASTNode *Node)
    {
        // if (auto Identifier = Cast<IdentifierNode>(Node))
        // {
        //     CTimeValue* Value = VisitIdentifier(Identifier);
        //     if (!Value)
        //         return nullptr;
        //
        //     //if (Value->Type->)
        // }
        // //if (auto Subscript )
        return nullptr;
    }

    bool TypeChecker::CanCastPointers(PointerType *Src, PointerType *Dst)
    {
        return Src == Dst || Dst->BaseType->GetCategory() == TypeCategory::VOID;
    }

    // bool TypeChecker::CanCastArithmetic(DataType* Left, DataType* Right, OperatorType Type) const
    // {
    //     TypeCategory LeftTypeCategory = Left->GetCategory();
    //     TypeCategory RightTypeCategory = Right->GetCategory();
    //
    //     switch (Type)
    //     {
    //         case OperatorType::ADD:
    //         case OperatorType::SUB:
    //         case OperatorType::MUL:
    //         case OperatorType::DIV:
    //         {
    //             switch (LeftTypeCategory)
    //             {
    //                 case TypeCategory::INTEGER:
    //                 {
    //                     switch (RightTypeCategory)
    //                     {
    //                         case TypeCategory::INTEGER:
    //                         case TypeCategory::FLOATING_POINT:
    //                             break;
    //                         case TypeCategory::POINTER:
    //                             if (Type == OperatorType::ADD)
    //                                 return true;
    //                             return false;
    //                         default:
    //                             return false;
    //                     }
    //                 }
    //
    //                 case TypeCategory::FLOATING_POINT:
    //                     return RightTypeCategory == TypeCategory::FLOATING_POINT ||
    //                         RightTypeCategory == TypeCategory::INTEGER;
    //
    //                 case TypeCategory::POINTER:
    //                 {
    //                     if (Type != OperatorType::ADD && Type != OperatorType::SUB)
    //                         return false;
    //
    //                     switch (RightTypeCategory)
    //                     {
    //                         case TypeCategory::INTEGER:
    //                             return true;
    //                         default:
    //                             return false;
    //                     }
    //                 }
    //                 default:
    //                     return false;
    //             }
    //         }
    //         case OperatorType::MOD:
    //             return LeftTypeCategory == TypeCategory::INTEGER &&
    //                 RightTypeCategory == TypeCategory::INTEGER;
    //
    //         default:
    //             return false;
    //     }
    // }
    //
    // bool TypeChecker::CanCastComparison(DataType* Left, DataType* Right, OperatorType Type) const
    // {
    //     TypeCategory LeftTypeCategory = Left->GetCategory();
    //     TypeCategory RightTypeCategory = Right->GetCategory();
    //
    //     switch (Type)
    //     {
    //         case OperatorType::EQ:
    //         case OperatorType::NEQ:
    //         {
    //             switch (LeftTypeCategory)
    //             {
    //                 case TypeCategory::BOOLEAN:
    //                     return RightTypeCategory == TypeCategory::BOOLEAN ||
    //                         RightTypeCategory == TypeCategory::INTEGER;
    //                 case TypeCategory::INTEGER:
    //                 case TypeCategory::FLOATING_POINT:
    //                     return RightTypeCategory == TypeCategory::INTEGER ||
    //                         RightTypeCategory == TypeCategory::FLOATING_POINT;
    //                 case TypeCategory::POINTER:
    //                     return RightTypeCategory == TypeCategory::POINTER;
    //                 default:
    //                     return false;
    //             }
    //         }
    //
    //         case OperatorType::LT:
    //         case OperatorType::LTE:
    //         case OperatorType::GT:
    //         case OperatorType::GTE:
    //         {
    //             switch (LeftTypeCategory)
    //             {
    //                 case TypeCategory::INTEGER:
    //                 case TypeCategory::FLOATING_POINT:
    //                     return RightTypeCategory == TypeCategory::INTEGER ||
    //                         RightTypeCategory == TypeCategory::FLOATING_POINT;
    //                 default:
    //                     return false;
    //             }
    //         }
    //
    //         default:
    //             return false;
    //     }
    // }
    //
    // bool TypeChecker::CanCastLogical(DataType* Left, DataType* Right, OperatorType Type) const
    // {
    //     DataType* BoolType = CContext.GetBoolType();
    //
    //     switch (Type)
    //     {
    //         case OperatorType::LOGICAL_AND:
    //         case OperatorType::LOGICAL_OR:
    //             return Left->ImplicitCast(BoolType) &&
    //                 Right->ImplicitCast(BoolType);
    //         default:
    //             return false;
    //     }
    // }
    //
    // bool TypeChecker::CanCastBitwise(DataType* Left, DataType* Right, OperatorType Type) const
    // {
    //     TypeCategory LeftTypeCategory = Left->GetCategory();
    //     TypeCategory RightTypeCategory = Right->GetCategory();
    //
    //     switch (Type)
    //     {
    //         case OperatorType::BIT_AND:
    //         case OperatorType::BIT_OR:
    //         case OperatorType::BIT_XOR:
    //             return (LeftTypeCategory == TypeCategory::INTEGER ||
    //                     LeftTypeCategory == TypeCategory::BOOLEAN) &&
    //                    (RightTypeCategory == TypeCategory::INTEGER ||
    //                     RightTypeCategory == TypeCategory::BOOLEAN);
    //
    //         case OperatorType::LSHIFT:
    //         case OperatorType::RSHIFT:
    //             return LeftTypeCategory == TypeCategory::INTEGER &&
    //                 RightTypeCategory == TypeCategory::INTEGER;
    //
    //         default:
    //             return false;
    //     }
    // }
    //
    // bool TypeChecker::CanCastAssignment(DataType* Left, DataType* Right, OperatorType Type) const
    // {
    //     switch (Type)
    //     {
    //         case OperatorType::ASSIGN:
    //             return Right->ImplicitCast(Left);
    //         case OperatorType::ADD_ASSIGN:
    //             return CanCastArithmetic(Right, Left, OperatorType::ADD);
    //         case OperatorType::SUB_ASSIGN:
    //             return CanCastArithmetic(Right, Left, OperatorType::SUB);
    //         case OperatorType::MUL_ASSIGN:
    //             return CanCastArithmetic(Right, Left, OperatorType::MUL);
    //         case OperatorType::DIV_ASSIGN:
    //             return CanCastArithmetic(Right, Left, OperatorType::DIV);
    //         default:
    //             return false;
    //     }
    // }
    //
    // bool TypeChecker::CanCastToJointType(DataType* Left, DataType* Right, OperatorType Type) const
    // {
    //     if (CanCastArithmetic(Left, Right, Type)) return true;
    //     if (CanCastComparison(Left, Right, Type)) return true;
    //     if (CanCastLogical(Left, Right, Type))    return true;
    //     if (CanCastBitwise(Left, Right, Type))    return true;
    //     if (CanCastAssignment(Left, Right, Type)) return true;
    //
    //     return false;
    // }

    // bool TypeChecker::CastToJointType(DataType *&Left, DataType *&Right, OperatorType Type, size_t Line, size_t Column)
    // {
    //     if (!CanCastToJointType(Left, Right, Type))
    //     {
    //         SendError(TypeErrorKind::InvalidBinaryOperator, Line, Column,{ Operator::ToString(Type),
    //            Left->ToString(), Left->ToString() });
    //         return false;
    //     }
    //
    //     int LeftTypeRank = Left->GetRank();
    //     int RightTypeRank = Right->GetRank();
    //
    //     if (LeftTypeRank == -1 || RightTypeRank == -1)
    //         return false;
    //
    //     if (LeftTypeRank == RightTypeRank)
    //         return true;
    //
    //     DataType*& Src = LeftTypeRank > RightTypeRank ? Right : Left;
    //     DataType*& Dst = LeftTypeRank > RightTypeRank ? Left : Right;
    //
    //     return ImplicitCastOrError(Src, Dst, Line, Column);
    // }

    bool TypeChecker::ImplicitCastOrError(DataType *&Src, DataType* Dst, size_t Line, size_t Column)
    {
        if (Src->ImplicitCast(Dst))
            return true;

        SendError(TypeErrorKind::IncompatibleTypes, Line, Column,
            { Src->ToString(), Dst->ToString() });
        return false;
    }

    /*bool TypeChecker::CastToJointType(CTimeValue *Left, CTimeValue *Right, OperatorType Type, size_t Line, size_t Column)
    {
        DataType* LeftType = Left->Type;
        DataType* RightType = Right->Type;

        if (!CanCastToJointType(LeftType, RightType, Type))
        {
            SendError(TypeErrorKind::InvalidBinaryOperator, Line, Column,{ Operator::ToString(Type),
                LeftType->ToString(), RightType->ToString() });
            return false;
        }

        int LeftTypeRank =  LeftType->GetRank();
        int RightTypeRank = RightType->GetRank();

        if (LeftTypeRank == -1 || RightTypeRank == -1)
            return false;

        if (LeftTypeRank == RightTypeRank)
            return true;

        CTimeValue* Src = LeftTypeRank > RightTypeRank ? Right : Left;
        DataType*& Dst = LeftTypeRank > RightTypeRank ? LeftType : RightType;

        return Src->ImplicitCast(Dst); //ImplicitCastOrError(Src, Dst, Line, Column);
    }*/

    bool TypeChecker::CanExplicitCast(DataType *Dst, DataType *Src)
    {
        if (Dst == Src) return true;

        if (Dst->ImplicitCast(Src))
            return true;

        if (auto DstPtrType = Cast<PointerType>(Dst))
            if (auto SrcPtrType = Cast<PointerType>(Src))
                return DstPtrType->BaseType->GetCategory() == TypeCategory::VOID;

        return false;
    }

    bool TypeChecker::ExplicitCast(DataType *&Dst, DataType *Src)
    {
        if (CanExplicitCast(Dst, Src))
        {
            Dst = Src;
            return true;
        }

        return false;
    }

    bool TypeChecker::ExplicitCast(CTimeValue *&Src, DataType *Dst)
    {
        if (Src->ImplicitCast(Dst))
            return true;

        if (auto SrcPtrType = Cast<PointerType>(Src->Type))
        {
            if (auto DstPtrType = Cast<PointerType>(Dst))
            {
                if (SrcPtrType->BaseType->GetCategory() == TypeCategory::VOID)
                {
                    Src->Type = Dst;
                    return true;
                }

                return false;
            }
        }

        return false;
    }

    void TypeChecker::EnterScope()
    {
        ScopeStack.Emplace();
    }

    void TypeChecker::ExitScope()
    {
        for (const ScopeEntry& Entry : ScopeStack.Back())
        {
            if (Entry.Previous)
                Variables[Entry.Name] = Entry.Previous;
            else
                Variables.erase(Entry.Name);
        }

        ScopeStack.Pop();
    }

    void TypeChecker::DeclareVariable(const std::string &Name, DataType* Type)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            ScopeStack.Back().Add({ Name, Iter->second });

        Variables[Name] = MainArena.Create<TypedValue>(Type);
        ScopeStack.Back().Add({ Name, nullptr });
    }

    DataType* TypeChecker::GetVariable(const std::string &Name)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            return Iter->second->GetDataType();

        return nullptr;
    }

    CTimeValue *TypeChecker::CalculateUnary(CTimeValue *Operand, OperatorType Type) const
    {
        if (!Operand)
            return nullptr;

        if (Operand->IsEmpty)
            return CTimeValue::CreateEmpty(Operand->Type, MainArena);

        TypeCategory OperandTypeCategory = Operand->Type->GetCategory();
        switch (Type)
        {
            case OperatorType::ADD:
            {
                switch (OperandTypeCategory)
                {
                    case TypeCategory::INTEGER:
                    case TypeCategory::FLOATING_POINT:
                        return Operand;
                    default:
                        return nullptr;
                }
            }
            case OperatorType::SUB:
            {
                switch (OperandTypeCategory)
                {
                    case TypeCategory::INTEGER:
                        return CTimeValue::CreateInteger(Operand->Type, -Operand->Int, MainArena);
                    case TypeCategory::FLOATING_POINT:
                        return CTimeValue::CreateFloat(Operand->Type, -Operand->Float, MainArena);
                    default:
                        return nullptr;
                }
            }
            case OperatorType::BIT_NOT:
            {
                if (OperandTypeCategory == TypeCategory::INTEGER)
                    return CTimeValue::CreateInteger(Operand->Type, ~Operand->Int, MainArena);
                return nullptr;
            }
            case OperatorType::LOGICAL_NOT:
            {
                if (OperandTypeCategory == TypeCategory::BOOLEAN)
                    return CTimeValue::CreateBool(Operand->Type, !Operand->Bool, MainArena);

                return nullptr;
            }
            default: return CTimeValue::CreateEmpty(Operand->Type, MainArena);
        }
    }
}
