//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_ASTNODES_H
#define CVOLT_ASTNODES_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Parser/Operators/Operator.h"
//#include "Volt/Core/Memory/BufferView.h"
#include "Volt/Compiler/CompileTime/CTimeValue.h"
#include "Volt/Core/Functions/Callee.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include <llvm/ADT/TinyPtrVector.h>
#include <string>
#include <vector>
#include <algorithm>

namespace Volt
{
    class ASTNode : public Object
    {
        GENERATED_BODY(ASTNode, Object)
    public:
        DataType* ResolvedType = nullptr;
        CTimeValue* CompileTimeValue = nullptr;
        size_t Pos, Line, Column;
        ASTNode(size_t Pos, size_t Line, size_t Column)
            : Pos(Pos), Line(Line), Column(Column) {}
    };

    class SequenceNode : public ASTNode
    {
        GENERATED_BODY(SequenceNode, ASTNode);
    public:
        std::vector<ASTNode*> Statements;
        SequenceNode() : ASTNode(0, 0, 0) {}
    };

    class BlockNode : public ASTNode
    {
        GENERATED_BODY(BlockNode, ASTNode);
    public:
        std::vector<ASTNode*> Statements;
        BlockNode(size_t Pos, size_t Line, size_t Column) :
            ASTNode(Pos, Line, Column) {}
    };

    class ErrorNode : public ASTNode
    {
        GENERATED_BODY(ErrorNode, ASTNode);
    public:
        ErrorNode(size_t Pos, size_t Line, size_t Column) :
            ASTNode(Pos, Line, Column) {}
    };

    class IdentifierNode : public ASTNode
    {
        GENERATED_BODY(IdentifierNode, ASTNode);
    public:
        llvm::StringRef Value;
        IdentifierNode(llvm::StringRef Value, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Value(Value) {}
    };

    class IntegerNode : public ASTNode
    {
        GENERATED_BODY(IntegerNode, ASTNode)
    public:
        enum IntType
        {
            BYTE, INT, LONG
        };

    public:
        IntType Type;
        Int64 Value;
        IntegerNode(IntType Type, Int64 Value, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Type(Type), Value(Value) {}
    };

    class FloatingPointNode : public ASTNode
    {
        GENERATED_BODY(FloatingPointNode, ASTNode)
    public:
        enum FPType
        {
            FLOAT, DOUBLE
        };

    public:
        FPType Type;
        double Value;
        FloatingPointNode(FPType Type, double Value, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Type(Type), Value(Value) {}
    };

    class BoolNode : public ASTNode
    {
        GENERATED_BODY(BoolNode, ASTNode)
    public:
        bool Value;
        BoolNode(bool Value, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Value(Value) {}
    };

    class CharNode : public ASTNode
    {
        GENERATED_BODY(CharNode, ASTNode)
    public:
        char Value;
        CharNode(char Value, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Value(Value) {}
    };

    class StringNode : public ASTNode
    {
        GENERATED_BODY(StringNode, ASTNode)
    public:
        llvm::StringRef Value;
        StringNode(llvm::StringRef Value, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Value(Value) {}
    };

    class ArrayNode : public ASTNode
    {
        GENERATED_BODY(ArrayNode, ASTNode)
    public:
        SmallVec16<ASTNode*> Elements;
        ArrayNode(size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column) {}

        void AddItem(ASTNode* El)
        {
            Elements.push_back(El);
        }
    };

    class RefNode : public ASTNode
    {
        GENERATED_BODY(RefNode, ASTNode)
    public:
        ASTNode* Target;
        RefNode(ASTNode* Target, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Target(Target) {}
    };

    class UnaryOpNode : public ASTNode
    {
        GENERATED_BODY(UnaryOpNode, ASTNode)
    public:
        OperatorType Type;
        ASTNode* Operand;
        UnaryOpNode(OperatorType Type, ASTNode* Operand,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Type(Type), Operand(Operand) {}
    };

    class PrefixOpNode : public UnaryOpNode
    {
        GENERATED_BODY(PreffixOpNode, UnaryOpNode)
    public:
        PrefixOpNode(OperatorType Type, ASTNode* Operand,
            size_t Pos, size_t Line, size_t Column)
            : UnaryOpNode(Type, Operand, Pos, Line, Column) {}
    };

    class SuffixOpNode : public UnaryOpNode
    {
        GENERATED_BODY(SuffixOpNode, UnaryOpNode)
    public:
        SuffixOpNode(OperatorType Type, ASTNode* Operand,
            size_t Pos, size_t Line, size_t Column)
            : UnaryOpNode(Type, Operand, Pos, Line, Column) {}
    };

    class BinaryOpNode : public ASTNode
    {
        GENERATED_BODY(BinaryOpNode, ASTNode)
    public:
        DataType* OperandsType = nullptr;

        OperatorType Type;
        ASTNode* Left;
        ASTNode* Right;
        BinaryOpNode(OperatorType Type, ASTNode* Left, ASTNode* Right,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Type(Type), Left(Left), Right(Right) {}
    };

    class ComparisonNode : public BinaryOpNode
    {
        GENERATED_BODY(EqualityNode, BinaryOpNode)
    public:
        ComparisonNode(OperatorType Type, ASTNode* Left, ASTNode* Right,
            size_t Pos, size_t Line, size_t Column)
            : BinaryOpNode(Type, Left, Right, Pos, Line, Column) {}
    };

    class LogicalNode : public BinaryOpNode
    {
        GENERATED_BODY(LogicalNode, BinaryOpNode)
    public:
        LogicalNode(OperatorType Type, ASTNode* Left, ASTNode* Right,
            size_t Pos, size_t Line, size_t Column)
            : BinaryOpNode(Type, Left, Right, Pos, Line, Column) {}
    };

    class AssignmentNode : public BinaryOpNode
    {
        GENERATED_BODY(AssignmentNode, BinaryOpNode)
    public:
        AssignmentNode(OperatorType Type, ASTNode* Left, ASTNode* Right,
            size_t Pos, size_t Line, size_t Column)
            : BinaryOpNode(Type, Left, Right, Pos, Line, Column) {}
    };

    class CallNode : public ASTNode
    {
        GENERATED_BODY(CallNode, ASTNode)
    public:
        CalleeBase* ResolvedCallee = nullptr;
        ASTNode* Callee;
        llvm::TinyPtrVector<ASTNode*> Arguments;
        CallNode(ASTNode* Callee, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Callee(Callee) {}
        void AddArgument(ASTNode* Arg)
        {
            Arguments.push_back(Arg);
        }
    };

    class SubscriptNode : public ASTNode
    {
        GENERATED_BODY(SubscriptNode, ASTNode)
    public:
        ASTNode* Target;
        ASTNode* Index;
        SubscriptNode(ASTNode* Target, ASTNode* Index,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Target(Target), Index(Index) {}
    };

    class DataTypeNodeBase : public ASTNode
    {
        GENERATED_BODY(DataTypeNodeBase, ASTNode)
    public:
        DataTypeNodeBase(size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column) {}
    };

    class PrimitiveTypeNode : public DataTypeNodeBase
    {
        GENERATED_BODY(PrimitiveTypeNode, DataTypeNodeBase)
    public:
        PrimitiveDataType* Type;
        PrimitiveTypeNode(PrimitiveDataType* Type, size_t Pos, size_t Line, size_t Column)
            : DataTypeNodeBase(Pos, Line, Column), Type(Type) {}
    };

    class DerivedTypeNode : public DataTypeNodeBase
    {
        GENERATED_BODY(DerivedTypeNode, DataTypeNodeBase)
    public:
        DataTypeNodeBase* BaseType;

        DerivedTypeNode(DataTypeNodeBase* BaseType, size_t Pos, size_t Line, size_t Column)
            : DataTypeNodeBase(Pos, Line, Column), BaseType(BaseType) {}
    };

    class PointerTypeNode : public DerivedTypeNode
    {
        GENERATED_BODY(PointerTypeNode, DerivedTypeNode)
    public:
        PointerTypeNode(DataTypeNodeBase* BaseType, size_t Pos, size_t Line, size_t Column)
            : DerivedTypeNode(BaseType, Pos, Line, Column) {}
    };

    class ReferenceTypeNode : public DerivedTypeNode
    {
        GENERATED_BODY(PointerTypeNode, DerivedTypeNode)
    public:
        ReferenceTypeNode(DataTypeNodeBase* BaseType, size_t Pos, size_t Line, size_t Column)
            : DerivedTypeNode(BaseType, Pos, Line, Column) {}
    };

    class DataTypeNode : public DataTypeNodeBase
    {
        GENERATED_BODY(DataTypeNode, DataTypeNodeBase)
    public:
        DataType* Type;
        DataTypeNode(DataType* Type, size_t Pos, size_t Line, size_t Column)
            : DataTypeNodeBase(Pos, Line, Column), Type(Type) {}
    };

    class ArrayTypeNode : public DerivedTypeNode
    {
        GENERATED_BODY(ArrayTypeNode, DerivedTypeNode)
    public:
        ASTNode* Length;
        ArrayTypeNode(DataTypeNodeBase* BaseType, ASTNode* Length, size_t Pos, size_t Line, size_t Column)
            : DerivedTypeNode(BaseType, Pos, Line, Column), Length(Length) {}
    };

    class VariableNode : public ASTNode
    {
        GENERATED_BODY(VariableNode, ASTNode)
    public:
        DataTypeNodeBase* Type;
        llvm::StringRef Name;
        ASTNode* Value;
        VariableNode(DataTypeNodeBase* Type, llvm::StringRef Name, ASTNode* Value,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Type(Type), Name(Name), Value(Value) {}
    };

    class ParamNode : public ASTNode
    {
        GENERATED_BODY(ParamNode, ASTNode)
    public:
        DataTypeNodeBase* Type;
        llvm::StringRef Name;
        ASTNode* DefaultValue;
        ParamNode(DataTypeNodeBase* Type, llvm::StringRef Name, ASTNode* Value,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Type(Type), Name(Name), DefaultValue(Value) {}
    };

    class FunctionNode : public ASTNode
    {
        GENERATED_BODY(FunctionNode, ASTNode)
    public:
        DataTypeNodeBase* ReturnType;
        llvm::StringRef Name;
        llvm::TinyPtrVector<ParamNode*> Params;
        ASTNode* Body = nullptr;
        FunctionNode(DataTypeNodeBase* Type, llvm::StringRef Name,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), ReturnType(Type), Name(Name) {}

        bool AddParam(ParamNode* Prm)
        {
            if (std::find_if(
            Params.begin(), Params.end(),
            [&Prm](const ParamNode* Value)
                {
                    return Prm->Name == Value->Name;
                }) != Params.end())
                    return false;

            if (!Prm->DefaultValue)
            {
                if (std::find_if(
            Params.begin(), Params.end(),
            [&Prm](const ParamNode* Value)
                {
                    return Value->DefaultValue;
                }) != Params.end())
                    return false;
            }

            Params.push_back(Prm);
            return true;
        }
    };

    class ReturnNode : public ASTNode
    {
        GENERATED_BODY(ReturnNode, ASTNode)
    public:
        ASTNode* ReturnValue;
        ReturnNode(ASTNode* ReturnValue, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), ReturnValue(ReturnValue) {}
    };

    class IfNode : public ASTNode
    {
        GENERATED_BODY(IfNode, ASTNode)
    public:
        ASTNode* Condition;
        ASTNode* Branch;
        ASTNode* ElseBranch;
        IfNode(ASTNode* Condition, ASTNode* Branch, ASTNode* ElseBranch,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Condition(Condition),
            Branch(Branch), ElseBranch(ElseBranch) {}
    };

    class WhileNode : public ASTNode
    {
        GENERATED_BODY(WhileNode, ASTNode)
    public:
        ASTNode* Condition;
        ASTNode* Branch;
        WhileNode(ASTNode* Condition, ASTNode* Branch,
            size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column),
            Condition(Condition), Branch(Branch) {}
    };

    class ForNode : public ASTNode
    {
        GENERATED_BODY(ForNode, ASTNode)
    public:
        ASTNode* Initialization;
        ASTNode* Condition;
        ASTNode* Iteration;
        ASTNode* Body;
        ForNode(ASTNode* Initialization, ASTNode* Condition, ASTNode* Iteration,
            ASTNode* Body, size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column), Initialization(Initialization),
            Condition(Condition), Iteration(Iteration), Body(Body) {}
    };

    class BreakNode : public ASTNode
    {
        GENERATED_BODY(BreakNode, ASTNode)
    public:
        BreakNode(size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column) {}
    };

    class ContinueNode : public ASTNode
    {
        GENERATED_BODY(ContinueNode, ASTNode)
    public:
        ContinueNode(size_t Pos, size_t Line, size_t Column)
            : ASTNode(Pos, Line, Column) {}
    };
}
#endif //CVOLT_ASTNODES_H