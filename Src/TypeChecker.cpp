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

    DataType TypeChecker::VisitNode(ASTNode *Node)
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

    DataType TypeChecker::VisitInt(IntegerNode *Int)
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
        return Int->ResolvedType;
    }

    DataType TypeChecker::VisitFloat(FloatingPointNode *Float)
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
        return Float->ResolvedType;
    }

    DataType TypeChecker::VisitBool(BoolNode *Bool)
    {
        Bool->ResolvedType = DataType::CreateBoolean(MainArena);
        return Bool->ResolvedType;
    }

    DataType TypeChecker::VisitChar(CharNode *Char)
    {
        Char->ResolvedType = DataType::CreateChar(MainArena);
        return Char->ResolvedType;
    }

    DataType TypeChecker::VisitString(StringNode *String)
    {
        String->ResolvedType = DataType::CreatePtr(DataType::CreateChar(MainArena), MainArena);
        return String->ResolvedType;
    }

    DataType TypeChecker::VisitArray(ArrayNode *Array)
    {
        llvm::ArrayRef<ASTNode*> Elements = Array->Elements;

        if (Elements.empty())
            return nullptr;

        DataType ElementsType = nullptr;
        bool HasErrors = false;
        for (auto El : Elements)
        {
            DataType ElType = VisitNode(El);
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

        Array->ResolvedType = DataType::CreatePtr(ElementsType.GetTypeBase(), MainArena);
        return Array->ResolvedType;
    }

    DataType TypeChecker::VisitIdentifier(IdentifierNode *Identifier)
    {
        DataType VarType = GetVariable(Identifier->Value.ToString());;
        if (!VarType)
            SendError(TypeErrorKind::UndefinedVariable, Identifier, { Identifier->Value.ToString() });

        Identifier->ResolvedType = VarType;
        return VarType;
    }

    DataType TypeChecker::VisitRef(RefNode *Ref)
    {
        DataType RefType = VisitNode(Ref->Target);
        if (!RefType)
            return nullptr;
        return DataType::CreatePtr(RefType, MainArena);
    }

    DataType TypeChecker::VisitSuffix(SuffixOpNode *Suffix)
    {
        DataType SuffixType = VisitNode(Suffix->Operand);
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
                    return SuffixType;
                }

                SendError(TypeErrorKind::InvalidUnaryOperator, Suffix,
                    { Operator::ToString(Suffix->Type), SuffixType.ToString() });
                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    DataType TypeChecker::VisitPrefix(PrefixOpNode *Prefix)
    {
        DataType PrefixType = VisitNode(Prefix->Operand);
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
                    return PrefixType;
                }
                SendError(TypeErrorKind::InvalidUnaryOperator, Prefix,
                    { Operator::ToString(Prefix->Type), PrefixType.ToString() });

                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    DataType TypeChecker::VisitUnary(UnaryOpNode *Unary)
    {
        DataType OperandType = VisitNode(Unary->Operand);
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
                    return OperandType;
                }
                return nullptr;
            }
            case Operator::LOGICAL_NOT:
            {
                if (ImplicitCast(OperandType, DataType::CreateBoolean(MainArena)))
                {
                    Unary->ResolvedType = OperandType;
                    return OperandType;
                }
                return nullptr;
            }
            case Operator::BIT_NOT:
            {
                if (OperandTypeCategory == TypeCategory::INTEGER)
                {
                    Unary->ResolvedType = OperandType;
                    return OperandType;
                }
                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    DataType TypeChecker::VisitComparison(ComparisonNode *Comparison)
    {
        DataType LeftType = VisitNode(Comparison->Left);
        DataType RightType = VisitNode(Comparison->Right);

        if (!LeftType || !RightType)
            return nullptr;

        if (CastToJointType(LeftType, RightType, Comparison->Type, Comparison->Line, Comparison->Column))
        {
            Comparison->ResolvedType = DataType::CreateBoolean(MainArena);
            Comparison->OperandsType = LeftType;
            return Comparison->ResolvedType;
        }

        return nullptr;
    }

    DataType TypeChecker::VisitBinary(BinaryOpNode *Binary)
    {
        DataType LeftType = VisitNode(Binary->Left);
        DataType RightType = VisitNode(Binary->Right);

        if (!LeftType || !RightType)
            return nullptr;

        if (CastToJointType(LeftType, RightType, Binary->Type, Binary->Line, Binary->Column))
        {
            Binary->ResolvedType = LeftType;
            Binary->OperandsType = LeftType;
            return LeftType;
        }

        return nullptr;
    }

    DataType TypeChecker::VisitCall(CallNode *Call)
    {
        if (auto Identifier = Cast<IdentifierNode>(Call->Callee))
        {
            const std::string& Name = Identifier->Value.ToString();
            llvm::SmallVector<DataType, 8> ArgTypes;
            ArgTypes.reserve(Call->Arguments.size());

            for (auto Arg : Call->Arguments)
            {
                DataType ArgType = VisitNode(Arg);
                if (!ArgType)
                    return nullptr;
                ArgTypes.push_back(ArgType);
            }

            FunctionSignature Signature(Name, ArgTypes);

            if (auto Iter = Functions.find(Signature); Iter != Functions.end())
            {
                Call->ResolvedType = Iter->second->GetReturnType();
                return Call->ResolvedType;
            }

            if (auto Func = BuiltinFuncTable.Get(Signature))
            {
                Call->ResolvedType = Func->ReturnType;
                return Func->ReturnType;
            }

            SendError(TypeErrorKind::UndefinedFunction, Call->Callee, { Name });
            return nullptr;
        }

        SendError(TypeErrorKind::InvalidCalleeType, Call->Callee);
        return nullptr;
    }

    DataType TypeChecker::VisitSubscript(SubscriptNode *Subscript)
    {
        DataType TargetType = VisitNode(Subscript->Target);
        DataType IndexType = VisitNode(Subscript->Index);

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

        if (auto PtrType = TargetType.GetPtrType())
        {
            Subscript->ResolvedType = PtrType->BaseType;
            return PtrType->BaseType;
        }

        return nullptr;
    }

    DataType TypeChecker::VisitVariable(VariableNode *Variable)
    {
        DataType VarType = DataType::CreateFromAST(Variable->Type, MainArena);
        DataType ValueType = VisitNode(Variable->Value);

        if (!VarType)
            return nullptr;

        if (ValueType && !ImplicitCast(ValueType, VarType))
        {
            SendError(TypeErrorKind::AssignmentTypeMismatch,
                Variable,{Variable->Name.ToString(),
                VarType.ToString(), ValueType.ToString()});
            return nullptr;
        }

        DeclareVariable(Variable->Name.ToString(), VarType);
        return nullptr;
    }

    DataType TypeChecker::VisitFunction(FunctionNode *Function)
    {
        llvm::SmallVector<DataType, 8> Params;
        Params.reserve(Function->Params.size());
        FunctionParams.reserve(Function->Params.size());
        for (const auto& Param : Function->Params)
        {
            DataType ParamType = DataType::CreateFromAST(Param->Type, MainArena);
            Params.push_back(ParamType);
            FunctionParams.emplace_back(Param->Name.ToString(), ParamType);
        }

        FunctionSignature Signature(Function->Name.ToString(), Params);
        DataType ReturnType = DataType::CreateFromAST(Function->ReturnType, MainArena);
        Functions[Signature] = MainArena.Create<TypedFunction>(ReturnType);

        FunctionReturnType = ReturnType;
        VisitBlock(Cast<BlockNode>(Function->Body));
        FunctionReturnType = nullptr;

        return nullptr;
    }

    DataType TypeChecker::VisitIf(IfNode *If)
    {
        DataType CondType = VisitNode(If->Condition);
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

    DataType TypeChecker::VisitWhile(WhileNode *While)
    {
        DataType CondType = VisitNode(While->Condition);
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

    DataType TypeChecker::VisitFor(ForNode *For)
    {
        VisitNode(For->Initialization);
        DataType CondType = VisitNode(For->Condition);
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

    DataType TypeChecker::VisitReturn(ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            if (FunctionReturnType == DataType(DataType::CreateVoid(MainArena)))
            {
                SendError(TypeErrorKind::VoidReturnValue, Return->ReturnValue);
                return nullptr;
            }

            DataType ReturnType = VisitNode(Return->ReturnValue);
            if (!CanImplicitCast(ReturnType, FunctionReturnType))
                SendError(TypeErrorKind::ReturnTypeMismatch, Return->ReturnValue);

            return nullptr;
        }

        if (FunctionReturnType != DataType(DataType::CreateVoid(MainArena)))
            SendError(TypeErrorKind::NonVoidMissingReturn, Return);

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

        return false;
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
}
