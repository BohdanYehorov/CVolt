//
// Created by bohdan on 15.01.26.
//

#include "Volt/Core/TypeChecker/TypeChecker.h"

namespace Volt
{
    llvm::DenseMap<TypeCategory, llvm::DenseSet<TypeCategory>> TypeChecker::ImplicitCastTypes = {
        { TypeCategory::BOOLEAN, {
            TypeCategory::BOOLEAN,
            TypeCategory::INTEGER
        } },
        { TypeCategory::INTEGER, {
            TypeCategory::BOOLEAN,
            TypeCategory::INTEGER,
            TypeCategory::FLOATING_POINT
        } },
        { TypeCategory::FLOATING_POINT, {
            TypeCategory::INTEGER,
            TypeCategory::FLOATING_POINT
        } },
        { TypeCategory::POINTER, {
            TypeCategory::BOOLEAN,
            TypeCategory::POINTER
        } }
    };

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
        if (auto Suffix = Cast<SuffixOpNode>(Node))
            return VisitSuffix(Suffix);
        if (auto Prefix = Cast<PrefixOpNode>(Node))
            return VisitPrefix(Prefix);
        if (auto Unary = Cast<UnaryOpNode>(Node))
            return VisitUnary(Unary);
        if (auto Comparison = Cast<ComparisonNode>(Node))
            return VisitComparison(Comparison);
        if (auto Binary = Cast<BinaryOpNode>(Node))
            return VisitBinary(Binary);
        if (auto Call = Cast<CallNode>(Node))
            return VisitCall(Call);
        if (auto Subscript = Cast<SubscriptNode>(Node))
            return VisitSubscript(Subscript);
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

        Int->ResolvedType = DataType::CreateInteger(BitWidth, MainArena);
        Int->CompileTimeValue = CTimeValue::CreateInteger(Int->ResolvedType, Int->Value, MainArena);
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

        Float->ResolvedType = DataType::CreateFloatingPoint(BitWidth, MainArena);
        Float->CompileTimeValue = CTimeValue::CreateFloat(Float->ResolvedType, Float->Value, MainArena);
        return Float->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitBool(BoolNode *Bool)
    {
        Bool->ResolvedType = DataType::CreateBoolean(MainArena);
        Bool->CompileTimeValue = CTimeValue::CreateBool(Bool->ResolvedType, Bool->Value, MainArena);
        return Bool->CompileTimeValue;
    }

    CTimeValue *TypeChecker::VisitChar(CharNode *Char)
    {
        Char->ResolvedType = DataType::CreateChar(MainArena);
        return CTimeValue::CreateChar(Char->ResolvedType, Char->Value, MainArena);
    }

    CTimeValue *TypeChecker::VisitString(StringNode *String)
    {
        String->ResolvedType = DataType::CreatePtr(DataType::CreateChar(MainArena), MainArena);
        return CTimeValue::CreatePointer(String->ResolvedType, String->Value.Data(), MainArena);
    }

    CTimeValue *TypeChecker::VisitArray(ArrayNode *Array)
    {
        llvm::ArrayRef<ASTNode*> Elements = Array->Elements;

        if (Elements.empty())
            return nullptr;

        DataType ElementsType = nullptr;
        bool HasErrors = false;
        for (auto El : Elements)
        {
            DataType ElType = VisitNode(El)->Type;
            if (!ElType)
                return nullptr;

            if (!ElementsType)
                ElementsType = ElType;
            else if (ElementsType != ElType)
            {
                SendError(TypeErrorKind::ArrayElementTypeMismatch,
                    El, { ElementsType.ToString(), ElType.ToString() });
                HasErrors = true;
            }
        }

        if (HasErrors)
            return nullptr;

        Array->ResolvedType = DataType::CreateArray(
            ElementsType.GetTypeBase(), Elements.size(), MainArena);
        return CTimeValue::CreateNull(Array->ResolvedType, MainArena);
    }

    CTimeValue *TypeChecker::VisitIdentifier(IdentifierNode *Identifier)
    {
        DataType VarType = GetVariable(Identifier->Value.ToString());;
        if (!VarType)
            SendError(TypeErrorKind::UndefinedVariable, Identifier, { Identifier->Value.ToString() });

        Identifier->ResolvedType = VarType;
        return CTimeValue::CreateNull(VarType, MainArena);
    }

    CTimeValue *TypeChecker::VisitRef(RefNode *Ref)
    {
        DataType RefType = VisitNode(Ref->Target)->Type;
        if (!RefType)
            return nullptr;
        return CTimeValue::CreateNull(DataType::CreatePtr(RefType, MainArena), MainArena);
    }

    CTimeValue *TypeChecker::VisitSuffix(SuffixOpNode *Suffix)
    {
        DataType SuffixType = VisitNode(Suffix->Operand)->Type;
        if (!SuffixType)
            return nullptr;

        switch (Suffix->Type)
        {
            case Operator::INC:
            case Operator::DEC:
            {
                if (SuffixType.GetTypeCategory() == TypeCategory::INTEGER)
                {
                    Suffix->ResolvedType = SuffixType;
                    return CTimeValue::CreateNull(SuffixType, MainArena);
                }

                SendError(TypeErrorKind::InvalidUnaryOperator, Suffix,
                    { Operator::ToString(Suffix->Type), SuffixType.ToString() });
                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    CTimeValue *TypeChecker::VisitPrefix(PrefixOpNode *Prefix)
    {
        DataType PrefixType = VisitNode(Prefix->Operand)->Type;
        if (!PrefixType)
            return nullptr;

        switch (Prefix->Type)
        {
            case Operator::INC:
            case Operator::DEC:
            {
                if (PrefixType.GetTypeCategory() == TypeCategory::INTEGER)
                {
                    Prefix->ResolvedType = PrefixType;
                    return CTimeValue::CreateNull(PrefixType,  MainArena);
                }
                SendError(TypeErrorKind::InvalidUnaryOperator, Prefix,
                    { Operator::ToString(Prefix->Type), PrefixType.ToString() });

                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    CTimeValue *TypeChecker::VisitUnary(UnaryOpNode *Unary)
    {
        CTimeValue* Operand = VisitNode(Unary->Operand);
        DataType OperandType = Operand->Type;
        if (!OperandType)
            return nullptr;

        TypeCategory OperandTypeCategory = OperandType.GetTypeCategory();

        switch (Unary->Type)
        {
            case Operator::ADD:
            case Operator::SUB:
            {
                if (OperandTypeCategory == TypeCategory::INTEGER ||
                   OperandTypeCategory == TypeCategory::FLOATING_POINT)
                {
                    Unary->ResolvedType = OperandType;
                    Unary->CompileTimeValue = CalculateUnary(Operand, Unary->Type);
                    return Unary->CompileTimeValue;
                }
                return nullptr;
            }
            case Operator::LOGICAL_NOT:
            {
                if (ImplicitCast(OperandType, DataType::CreateBoolean(MainArena)))
                {
                    Unary->ResolvedType = OperandType;
                    Unary->CompileTimeValue = CalculateUnary(Operand, Unary->Type);
                    return Unary->CompileTimeValue;
                }
                return nullptr;
            }
            case Operator::BIT_NOT:
            {
                if (OperandTypeCategory == TypeCategory::INTEGER)
                {
                    Unary->ResolvedType = OperandType;
                    Unary->CompileTimeValue = CalculateUnary(Operand, Unary->Type);
                    return Unary->CompileTimeValue;
                }
                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    CTimeValue *TypeChecker::VisitComparison(ComparisonNode *Comparison)
    {
        CTimeValue* Left = VisitNode(Comparison->Left);
        CTimeValue* Right = VisitNode(Comparison->Right);

        DataType LeftType = Left->Type;
        DataType RightType = Right->Type;

        if (!LeftType || !RightType)
            return nullptr;

        if (CastToJointType(Left, Right, Comparison->Type, Comparison->Line, Comparison->Column))
        {
            Comparison->ResolvedType = DataType::CreateBoolean(MainArena);
            Comparison->OperandsType = LeftType;
            Comparison->CompileTimeValue = CalculateBinary(Left, Right, Comparison->Type);
            return Comparison->CompileTimeValue;
        }

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitBinary(BinaryOpNode *Binary)
    {
        CTimeValue* Left = VisitNode(Binary->Left);
        CTimeValue* Right = VisitNode(Binary->Right);

        DataType LeftType = Left->Type;
        DataType RightType = Right->Type;

        if (!LeftType || !RightType)
            return nullptr;

        if (CastToJointType(Left, Right, Binary->Type, Binary->Line, Binary->Column))
        {
            Binary->ResolvedType = LeftType;
            Binary->OperandsType = LeftType;
            Binary->CompileTimeValue = CalculateBinary(Left, Right, Binary->Type);
            return Binary->CompileTimeValue;
        }

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitCall(CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            const std::string& Name = Identifier->Value.ToString();
            llvm::SmallVector<DataType, 8> ArgTypes;
            ArgTypes.reserve(Call->Arguments.size());

            for (auto Arg : Call->Arguments)
            {
                DataType ArgType = VisitNode(Arg)->Type;
                if (!ArgType)
                    return nullptr;
                ArgTypes.push_back(ArgType);
            }

            FunctionSignature Signature(Name, ArgTypes);

            if (auto Iter = Functions.find(Signature); Iter != Functions.end())
            {
                Call->ResolvedType = Iter->second->GetReturnType();
                return CTimeValue::CreateNull(Call->ResolvedType, MainArena);
            }

            if (auto Func = BuiltinFuncTable.Get(Signature))
            {
                Call->ResolvedType = Func->ReturnType;
                return CTimeValue::CreateNull(Func->ReturnType, MainArena);
            }

            SendError(TypeErrorKind::UndefinedFunction, Call->Callee, { Name });
            return nullptr;
        }

        SendError(TypeErrorKind::InvalidCalleeType, Call->Callee);
        return nullptr;
    }

    CTimeValue *TypeChecker::VisitSubscript(SubscriptNode *Subscript)
    {
        DataType TargetType = VisitNode(Subscript->Target)->Type;
        DataType IndexType = VisitNode(Subscript->Index)->Type;

        DataType Int32Type = DataType::CreateInteger(32, MainArena);
        // if (!CanImplicitCast(IndexType, Int32Type))
        // {
        //     SendError(TypeErrorKind::TypeMissmatch,
        //         Subscript->Index, { IndexType.ToString(), Int32Type.ToString() });
        //     return nullptr;
        // }

        if (!ImplicitCastOrError(IndexType, Int32Type, Subscript->Index->Line, Subscript->Index->Column))
            return nullptr;

        Subscript->Index->ResolvedType = IndexType;

        if (auto PtrType = TargetType.GetArrayType())
        {
            Subscript->ResolvedType = PtrType->BaseType;
            return CTimeValue::CreateNull(PtrType->BaseType, MainArena);
        }

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitVariable(VariableNode *Variable)
    {
        DataType VarType = VisitType(Variable->Type);
        CTimeValue* Value = VisitNode(Variable->Value);

        if (!VarType)
            return nullptr;

        if (Value && !ImplicitCast(Value->Type, VarType))
        {
            SendError(TypeErrorKind::AssignmentTypeMismatch,
                Variable,{Variable->Name.ToString(),
                VarType.ToString(), Value->Type.ToString()});
            return nullptr;
        }

        DeclareVariable(Variable->Name.ToString(), VarType);
        return nullptr;
    }

    CTimeValue *TypeChecker::VisitFunction(FunctionNode *Function)
    {
        llvm::SmallVector<DataType, 8> Params;
        Params.reserve(Function->Params.size());
        FunctionParams.reserve(Function->Params.size());
        for (const auto& Param : Function->Params)
        {
            DataType ParamType = VisitType(Param->Type);
            Params.push_back(ParamType);
            FunctionParams.emplace_back(Param->Name.ToString(), ParamType);
        }

        FunctionSignature Signature(Function->Name.ToString(), Params);
        DataType ReturnType = VisitType(Function->ReturnType);
        Functions[Signature] = MainArena.Create<TypedFunction>(ReturnType);

        FunctionReturnType = ReturnType;
        VisitBlock(Cast<BlockNode>(Function->Body));
        FunctionReturnType = nullptr;

        return nullptr;
    }

    CTimeValue *TypeChecker::VisitIf(IfNode *If)
    {
        DataType CondType = VisitNode(If->Condition)->Type;
        if (!CondType)
            return nullptr;

        if (!CanImplicitCast(CondType, DataType::CreateBoolean(MainArena)))
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
        DataType CondType = VisitNode(While->Condition)->Type;
        if (!CondType)
            return nullptr;

        if (!CanImplicitCast(CondType, DataType::CreateBoolean(MainArena)))
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
        DataType CondType = VisitNode(For->Condition)->Type;
        if (!CondType)
            return nullptr;

        if (!CanImplicitCast(CondType, DataType::CreateBoolean(MainArena)))
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
            if (FunctionReturnType == DataType(DataType::CreateVoid(MainArena)))
            {
                SendError(TypeErrorKind::VoidReturnValue, Return->ReturnValue);
                return nullptr;
            }

            DataType ReturnType = VisitNode(Return->ReturnValue)->Type;
            if (!CanImplicitCast(ReturnType, FunctionReturnType))
                SendError(TypeErrorKind::ReturnTypeMismatch, Return->ReturnValue);

            return nullptr;
        }

        if (FunctionReturnType != DataType(DataType::CreateVoid(MainArena)))
            SendError(TypeErrorKind::NonVoidMissingReturn, Return);

        return nullptr;
    }

    DataType TypeChecker::VisitType(DataTypeNodeBase *Type)
    {
        if (auto Primitive = Cast<PrimitiveTypeNode>(Type))
        {
            Primitive->ResolvedType = Primitive->Type;
            return Primitive->Type;
        }
        if (auto Ptr = Cast<PointerTypeNode>(Type))
        {
            Ptr->ResolvedType = DataType::CreatePtr(VisitType(Ptr->BaseType), MainArena);
            return Ptr->ResolvedType;
        }
        if (auto Array = Cast<ArrayTypeNode>(Type))
        {
            CTimeValue* Length = VisitNode(Array->Length);
            if (Length && Length->Type.GetTypeCategory() == TypeCategory::INTEGER)
            {
                Array->ResolvedType = DataType::CreateArray(VisitType(Array->BaseType), Length->Int, MainArena);
                return Array->ResolvedType;
            }

            throw std::runtime_error("Array length mast be defined in compiler time");
        }

        return nullptr;
    }

    bool TypeChecker::CanImplicitCast(DataType Src, DataType Dst) const
    {
        if (Src == Dst) return true;

        TypeCategory SrcTypeCategory = Src.GetTypeCategory();
        TypeCategory DstTypeCategory = Dst.GetTypeCategory();

        if (auto SrcIter = ImplicitCastTypes.find(SrcTypeCategory); SrcIter != ImplicitCastTypes.end())
        {
            const auto& DstImplicitCastTypes = SrcIter->second;
            if (auto DstIter = DstImplicitCastTypes.find(DstTypeCategory); DstIter != DstImplicitCastTypes.end())
                return true;
        }

        return false;
    }

    bool TypeChecker::CanCastArithmetic(DataType Left, DataType Right, Operator::Type Type) const
    {
        TypeCategory LeftTypeCategory = Left.GetTypeCategory();
        TypeCategory RightTypeCategory = Right.GetTypeCategory();

        switch (Type)
        {
            case Operator::ADD:
            case Operator::SUB:
            case Operator::MUL:
            case Operator::DIV:
            {
                switch (LeftTypeCategory)
                {
                    case TypeCategory::INTEGER:
                    {
                        switch (RightTypeCategory)
                        {
                            case TypeCategory::INTEGER:
                            case TypeCategory::FLOATING_POINT:
                                break;
                            case TypeCategory::POINTER:
                                if (Type == Operator::ADD)
                                    return true;
                                return false;
                            default:
                                return false;
                        }
                    }

                    case TypeCategory::FLOATING_POINT:
                        return RightTypeCategory == TypeCategory::FLOATING_POINT ||
                            RightTypeCategory == TypeCategory::INTEGER;

                    case TypeCategory::POINTER:
                    {
                        if (Type != Operator::ADD && Type != Operator::SUB)
                            return false;

                        switch (RightTypeCategory)
                        {
                            case TypeCategory::INTEGER:
                                return true;
                            default:
                                return false;
                        }
                    }
                    default:
                        return false;
                }
            }
            case Operator::MOD:
                return LeftTypeCategory == TypeCategory::INTEGER &&
                    RightTypeCategory == TypeCategory::INTEGER;

            default:
                return false;
        }
    }

    bool TypeChecker::CanCastComparison(DataType Left, DataType Right, Operator::Type Type) const
    {
        TypeCategory LeftTypeCategory = Left.GetTypeCategory();
        TypeCategory RightTypeCategory = Right.GetTypeCategory();

        switch (Type)
        {
            case Operator::EQ:
            case Operator::NEQ:
            {
                switch (LeftTypeCategory)
                {
                    case TypeCategory::BOOLEAN:
                        return RightTypeCategory == TypeCategory::BOOLEAN ||
                            RightTypeCategory == TypeCategory::INTEGER;
                    case TypeCategory::INTEGER:
                    case TypeCategory::FLOATING_POINT:
                        return RightTypeCategory == TypeCategory::INTEGER ||
                            RightTypeCategory == TypeCategory::FLOATING_POINT;
                    case TypeCategory::POINTER:
                        return RightTypeCategory == TypeCategory::POINTER;
                    default:
                        return false;
                }
            }

            case Operator::LT:
            case Operator::LTE:
            case Operator::GT:
            case Operator::GTE:
            {
                switch (LeftTypeCategory)
                {
                    case TypeCategory::INTEGER:
                    case TypeCategory::FLOATING_POINT:
                        return RightTypeCategory == TypeCategory::INTEGER ||
                            RightTypeCategory == TypeCategory::FLOATING_POINT;
                    default:
                        return false;
                }
            }

            default:
                return false;
        }
    }

    bool TypeChecker::CanCastLogical(DataType Left, DataType Right, Operator::Type Type) const
    {
        DataType BoolType = DataType::CreateBoolean(MainArena);

        switch (Type)
        {
            case Operator::LOGICAL_AND:
            case Operator::LOGICAL_OR:
                return CanImplicitCast(Left, BoolType) &&
                    CanImplicitCast(Right, BoolType);
            default:
                return false;
        }
    }

    bool TypeChecker::CanCastBitwise(DataType Left, DataType Right, Operator::Type Type) const
    {
        TypeCategory LeftTypeCategory = Left.GetTypeCategory();
        TypeCategory RightTypeCategory = Right.GetTypeCategory();

        switch (Type)
        {
            case Operator::BIT_AND:
            case Operator::BIT_OR:
            case Operator::BIT_XOR:
                return (LeftTypeCategory == TypeCategory::INTEGER ||
                        LeftTypeCategory == TypeCategory::BOOLEAN) &&
                       (RightTypeCategory == TypeCategory::INTEGER ||
                        RightTypeCategory == TypeCategory::BOOLEAN);

            case Operator::LSHIFT:
            case Operator::RSHIFT:
                return LeftTypeCategory == TypeCategory::INTEGER &&
                    RightTypeCategory == TypeCategory::INTEGER;

            default:
                return false;
        }
    }

    bool TypeChecker::CanCastAssignment(DataType Left, DataType Right, Operator::Type Type) const
    {
        switch (Type)
        {
            case Operator::ASSIGN:
                return CanImplicitCast(Right, Left);
            case Operator::ADD_ASSIGN:
                return CanCastArithmetic(Right, Left, Operator::ADD);
            case Operator::SUB_ASSIGN:
                return CanCastArithmetic(Right, Left, Operator::SUB);
            case Operator::MUL_ASSIGN:
                return CanCastArithmetic(Right, Left, Operator::MUL);
            case Operator::DIV_ASSIGN:
                return CanCastArithmetic(Right, Left, Operator::DIV);
            default:
                return false;
        }
    }

    bool TypeChecker::CanCastToJointType(DataType Left, DataType Right, Operator::Type Type) const
    {
        if (CanCastArithmetic(Left, Right, Type)) return true;
        if (CanCastComparison(Left, Right, Type)) return true;
        if (CanCastLogical(Left, Right, Type))    return true;
        if (CanCastBitwise(Left, Right, Type))    return true;
        if (CanCastAssignment(Left, Right, Type)) return true;

        return false;
    }

    bool TypeChecker::CastToJointType(DataType &Left, DataType &Right, Operator::Type Type, size_t Line, size_t Column)
    {
        if (!CanCastToJointType(Left, Right, Type))
        {
            SendError(TypeErrorKind::InvalidBinaryOperator,
                Line, Column, { Operator::ToString(Type), Left.ToString(), Right.ToString() });
            return false;
        }

        int LeftTypeRank = Left.GetTypeRank(MainArena);
        int RightTypeRank = Right.GetTypeRank(MainArena);

        if (LeftTypeRank == -1 || RightTypeRank == -1)
            return false;

        if (LeftTypeRank == RightTypeRank)
            return true;

        DataType& Src = LeftTypeRank > RightTypeRank ? Right : Left;
        DataType& Dst = LeftTypeRank > RightTypeRank ? Left : Right;

        return ImplicitCastOrError(Src, Dst, Line, Column);
    }

    bool TypeChecker::ImplicitCast(DataType &Src, DataType Dst)
    {
        if (Src == Dst) return true;

        TypeCategory SrcTypeCategory = Src.GetTypeCategory();
        TypeCategory DstTypeCategory = Dst.GetTypeCategory();

        if (auto SrcIter = ImplicitCastTypes.find(SrcTypeCategory); SrcIter != ImplicitCastTypes.end())
        {
            const auto& DstImplicitCastTypes = SrcIter->second;
            if (auto DstIter = DstImplicitCastTypes.find(DstTypeCategory); DstIter != DstImplicitCastTypes.end())
            {
                Src = Dst;
                return true;
            }
        }

        return false;
    }

    bool TypeChecker::ImplicitCastOrError(DataType &Src, DataType Dst, size_t Line, size_t Column)
    {
        if (ImplicitCast(Src, Dst))
            return true;

        SendError(TypeErrorKind::IncompatibleTypes, Line, Column, { Src.ToString(), Dst.ToString() });
        return false;
    }

    bool TypeChecker::ImplicitCast(CTimeValue *Src, DataType DstType)
    {
        DataType SrcType = Src->Type;

        if (SrcType == DstType) return true;

        TypeCategory SrcTypeCategory = SrcType.GetTypeCategory();
        TypeCategory DstTypeCategory = DstType.GetTypeCategory();

        if (auto SrcIter = ImplicitCastTypes.find(SrcTypeCategory); SrcIter != ImplicitCastTypes.end())
        {
            const auto& DstImplicitCastTypes = SrcIter->second;
            if (auto DstIter = DstImplicitCastTypes.find(DstTypeCategory); DstIter != DstImplicitCastTypes.end())
            {
                Src->Type = DstType;

                switch (DstTypeCategory)
                {
                    case TypeCategory::INTEGER:
                    {
                        switch (SrcTypeCategory)
                        {
                            case TypeCategory::INTEGER:
                                break;
                            case TypeCategory::FLOATING_POINT:
                                Src->Int = static_cast<UInt64>(Src->Float);
                                break;
                            case TypeCategory::BOOLEAN:
                                Src->Int = static_cast<UInt64>(Src->Bool);
                                break;
                            default:
                                break;
                        }
                        break;
                    }

                    case TypeCategory::FLOATING_POINT:
                    {
                        switch (SrcTypeCategory)
                        {
                            case TypeCategory::INTEGER:
                                Src->Float = static_cast<double>(Src->Int);
                                break;
                            case TypeCategory::FLOATING_POINT:
                                break;
                            case TypeCategory::BOOLEAN:
                                Src->Float = static_cast<double>(Src->Bool);
                                break;
                            default:
                                break;
                        }
                        break;
                    }

                    case TypeCategory::BOOLEAN:
                    {
                        switch (SrcTypeCategory)
                        {
                            case TypeCategory::INTEGER:
                                Src->Bool = static_cast<bool>(Src->Int);
                                break;
                            case TypeCategory::FLOATING_POINT:
                                Src->Bool = static_cast<bool>(Src->Float);
                                break;
                            case TypeCategory::BOOLEAN:
                                break;
                            default:
                                break;
                        }
                        break;
                    }

                    default:
                        break;
                }

                return true;
            }
        }

        return false;
    }

    bool TypeChecker::CastToJointType(CTimeValue *Left, CTimeValue *Right, Operator::Type Type, size_t Line, size_t Column)
    {
        DataType LeftType = Left->Type;
        DataType RightType = Right->Type;

        if (!CanCastToJointType(LeftType, RightType, Type))
        {
            SendError(TypeErrorKind::InvalidBinaryOperator,
                Line, Column, { Operator::ToString(Type), LeftType.ToString(), RightType.ToString() });
            return false;
        }

        int LeftTypeRank = LeftType.GetTypeRank(MainArena);
        int RightTypeRank = RightType.GetTypeRank(MainArena);

        if (LeftTypeRank == -1 || RightTypeRank == -1)
            return false;

        if (LeftTypeRank == RightTypeRank)
            return true;

        CTimeValue* Src = LeftTypeRank > RightTypeRank ? Right : Left;
        DataType& Dst = LeftTypeRank > RightTypeRank ? LeftType : RightType;

        return ImplicitCast(Src, Dst); //ImplicitCastOrError(Src, Dst, Line, Column);
    }

    void TypeChecker::EnterScope()
    {
        ScopeStack.emplace_back();
    }

    void TypeChecker::ExitScope()
    {
        for (const ScopeEntry& Entry : ScopeStack.back())
        {
            if (Entry.Previous)
                Variables[Entry.Name] = Entry.Previous;
            else
                Variables.erase(Entry.Name);
        }

        ScopeStack.pop_back();
    }

    void TypeChecker::DeclareVariable(const std::string &Name, DataType Type)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            ScopeStack.back().push_back({ Name, Iter->second });

        Variables[Name] = MainArena.Create<TypedValue>(Type);
        ScopeStack.back().push_back({ Name, nullptr });
    }

    DataType TypeChecker::GetVariable(const std::string &Name)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            return Iter->second->GetDataType();

        return nullptr;
    }

    CTimeValue *TypeChecker::CalculateUnary(CTimeValue *Operand, Operator::Type Type) const
    {
        if (!Operand || !Operand->IsValid)
            return nullptr;

        TypeCategory OperandTypeCategory = Operand->Type.GetTypeCategory();
        switch (Type)
        {
            case Operator::ADD:
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
            case Operator::SUB:
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
            case Operator::BIT_NOT:
            {
                if (OperandTypeCategory == TypeCategory::INTEGER)
                    return CTimeValue::CreateInteger(Operand->Type, ~Operand->Int, MainArena);
                return nullptr;
            }
            case Operator::LOGICAL_NOT:
            {
                if (OperandTypeCategory == TypeCategory::BOOLEAN)
                    return CTimeValue::CreateBool(Operand->Type, !Operand->Bool, MainArena);

                return nullptr;
            }
            default:
                return nullptr;
        }
    }

#define CREATE_OP_FOR_ALL_TYPES(Op) switch (Left->Type.GetTypeCategory()) \
    { \
        case TypeCategory::INTEGER: \
            return CTimeValue::CreateInteger(Left->Type, Left->Int Op Right->Int, MainArena); \
        case TypeCategory::FLOATING_POINT: \
            return CTimeValue::CreateFloat(Left->Type, Left->Float Op Right->Float, MainArena); \
        case TypeCategory::BOOLEAN: \
            return CTimeValue::CreateBool(Left->Type, Left->Bool Op Right->Bool, MainArena); \
        default: return nullptr; \
    }

#define CREATE_CMP_FOR_ALL_TYPES(Op) switch (Left->Type.GetTypeCategory()) \
    { \
        case TypeCategory::INTEGER: \
            return CTimeValue::CreateBool(DataType::CreateBoolean(MainArena), Left->Int Op Right->Int, MainArena); \
        case TypeCategory::FLOATING_POINT: \
            return CTimeValue::CreateBool(DataType::CreateBoolean(MainArena), Left->Float Op Right->Float, MainArena); \
        case TypeCategory::BOOLEAN: \
            return CTimeValue::CreateBool(Left->Type, Left->Bool Op Right->Bool, MainArena); \
        default: return nullptr; \
    }

#define CREATE_OP_FOR_INT(Op) switch (Left->Type.GetTypeCategory()) \
    { \
    case TypeCategory::INTEGER: \
        return CTimeValue::CreateInteger(Left->Type, Left->Int Op Right->Int, MainArena); \
    default: return nullptr; \
    }

#define CREATE_OP_FOR_BOOL(Op) switch (Left->Type.GetTypeCategory()) \
    { \
        case TypeCategory::BOOLEAN: \
            return CTimeValue::CreateBool(Left->Type, Left->Bool Op Right->Bool, MainArena); \
        default: return nullptr; \
    }

    CTimeValue *TypeChecker::CalculateBinary(CTimeValue *Left, CTimeValue *Right, Operator::Type Type) const
    {
        if (!Left->IsValid || !Right->IsValid)
            return nullptr;

        switch (Type)
        {
            case Operator::ADD:         CREATE_OP_FOR_ALL_TYPES(+);
            case Operator::SUB:         CREATE_OP_FOR_ALL_TYPES(-);
            case Operator::MUL:         CREATE_OP_FOR_ALL_TYPES(*);
            case Operator::DIV:         CREATE_OP_FOR_ALL_TYPES(/);
            case Operator::MOD:         CREATE_OP_FOR_INT(%);
            case Operator::EQ:          CREATE_CMP_FOR_ALL_TYPES(==);
            case Operator::NEQ:         CREATE_CMP_FOR_ALL_TYPES(!=);
            case Operator::GT:          CREATE_CMP_FOR_ALL_TYPES(>);
            case Operator::GTE:         CREATE_CMP_FOR_ALL_TYPES(>=);
            case Operator::LT:          CREATE_CMP_FOR_ALL_TYPES(<);
            case Operator::LTE:         CREATE_CMP_FOR_ALL_TYPES(<=);
            case Operator::LOGICAL_AND: CREATE_OP_FOR_BOOL(&&);
            case Operator::LOGICAL_OR:  CREATE_OP_FOR_BOOL(||);
            case Operator::BIT_AND:     CREATE_OP_FOR_INT(&);
            case Operator::BIT_OR:      CREATE_OP_FOR_INT(|);
            case Operator::BIT_XOR:     CREATE_OP_FOR_INT(^);
            case Operator::LSHIFT:      CREATE_OP_FOR_INT(<<);
            case Operator::RSHIFT:      CREATE_OP_FOR_INT(>>);
            default:                    return nullptr;
        }
    }
}
