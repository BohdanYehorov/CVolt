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

    DataType TypeChecker::VisitIdentifier(IdentifierNode *Identifier)
    {
        Identifier->ResolvedType = GetVariable(Identifier->Value.ToString());
        return Identifier->ResolvedType;
    }

    DataType TypeChecker::VisitSuffix(SuffixOpNode *Suffix)
    {
        DataType SuffixType = VisitNode(Suffix->Operand);

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

                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    DataType TypeChecker::VisitPrefix(PrefixOpNode *Prefix)
    {
        DataType PrefixType = VisitNode(Prefix->Operand);

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

                return nullptr;
            }
            default:
                return nullptr;
        }
    }

    DataType TypeChecker::VisitUnary(UnaryOpNode *Unary)
    {
        DataType OperandType = VisitNode(Unary->Operand);
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

    DataType TypeChecker::VisitBinary(BinaryOpNode *Binary)
    {
        DataType LeftType = VisitNode(Binary->Left);
        DataType RightType = VisitNode(Binary->Right);

        if (CastToJointType(LeftType, RightType, Binary->Type))
        {
            Binary->ResolvedType = LeftType;
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
                ArgTypes.push_back(VisitNode(Arg));

            FunctionSignature Signature(Name, ArgTypes);

            if (auto Iter = Functions.find(Signature); Iter != Functions.end())
            {
                Call->ResolvedType = Iter->second;
                return Iter->second;
            }

            int BestRank = std::numeric_limits<int>::max();
            for (const auto& [FuncSignature, FuncType] : Functions)
            {
                if (Name != FuncSignature.Name || ArgTypes.size() != FuncSignature.Params.size())
                    continue;


            }
        }

        return nullptr;
    }

    DataType TypeChecker::VisitVariable(VariableNode *Variable)
    {
        DataType VarType = Variable->Type->Type;
        DataType ValueType = VisitNode(Variable->Value);

        if (!ImplicitCast(ValueType, VarType))
            return nullptr;

        DeclareVariable(Variable->Name.ToString(),Variable->Type->Type);
        return nullptr;
    }

    DataType TypeChecker::VisitFunction(FunctionNode *Function)
    {
        llvm::SmallVector<DataType, 8> Params;
        Params.reserve(Function->Params.size());
        for (const auto& Param : Function->Params)
            Params.push_back(Param->Type->Type);

        FunctionSignature Signature(Function->Name.ToString(), Params);
        Functions[Signature] = Function->ReturnType->Type;

        FunctionReturnType = Function->ReturnType->Type;
        VisitBlock(Cast<BlockNode>(Function->Body));
        FunctionReturnType = nullptr;

        return nullptr;
    }

    DataType TypeChecker::VisitIf(IfNode *If)
    {
        DataType CondType = VisitNode(If->Condition);
        if (!CanImplicitCast(CondType, DataType::CreateBoolean(MainArena)))
            return nullptr;

        VisitNode(If->Branch);

        if (If->ElseBranch)
            VisitNode(If->ElseBranch);

        return nullptr;
    }

    DataType TypeChecker::VisitWhile(WhileNode *While)
    {
        DataType CondType = VisitNode(While->Condition);
        if (!CanImplicitCast(CondType, DataType::CreateBoolean(MainArena)))
            return nullptr;

        VisitNode(While->Branch);
        return nullptr;
    }

    DataType TypeChecker::VisitFor(ForNode *For)
    {
        VisitNode(For->Initialization);
        DataType CondType = VisitNode(For->Condition);
        if (!CanImplicitCast(CondType, DataType::CreateBoolean(MainArena)))
            return nullptr;
        VisitNode(For->Iteration);
        VisitNode(For->Body);

        return nullptr;
    }

    DataType TypeChecker::VisitReturn(ReturnNode *Return)
    {
        if (Return->ReturnValue)
        {
            DataType ReturnType = VisitNode(Return->ReturnValue);
            if (!CanImplicitCast(ReturnType, FunctionReturnType))
            { /*ERROR*/ }

            return nullptr;
        }

        if (FunctionReturnType != DataType(DataType::CreateVoid(MainArena)))
        { /*ERROR*/ }

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
                        if (Type != Operator::ADD || Type != Operator::SUB)
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

        return false;
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
        }
        return false;
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

    bool TypeChecker::CastToJointType(DataType &Left, DataType &Right, Operator::Type Type)
    {
        if (!CanCastToJointType(Left, Right, Type))
            return false;

        int LeftTypeRank = Left.GetTypeRank(MainArena);
        int RightTypeRank = Right.GetTypeRank(MainArena);

        if (LeftTypeRank == -1 || RightTypeRank == -1)
            return false;

        if (LeftTypeRank == RightTypeRank)
            return true;

        DataType& Src = LeftTypeRank > RightTypeRank ? Right : Left;
        DataType& Dst = LeftTypeRank > RightTypeRank ? Left : Right;

        return ImplicitCast(Src, Dst);
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

    void TypeChecker::EnterScope()
    {
        ScopeStack.emplace_back();
    }

    void TypeChecker::ExitScope()
    {
        for (const TypeScopeEntry& Entry : ScopeStack.back())
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

        Variables[Name] = Type;
        ScopeStack.back().push_back({ Name, nullptr });
    }

    DataType TypeChecker::GetVariable(const std::string &Name)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            return Iter->second;

        return nullptr;
    }
}
