//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_ASTNODES_H
#define CVOLT_ASTNODES_H

#include "Object.h"
#include <string>
#include <vector>
#include "Operator.h"
#include <algorithm>
#include "BufferView.h"
#include "DataType.h"
#include <llvm/ADT/TinyPtrVector.h>

class ASTNode : public Object
{
    GENERATED_BODY(ASTNode, Object)
};

class SequenceNode : public ASTNode
{
    GENERATED_BODY(SequenceNode, ASTNode);
public:
    std::vector<ASTNode*> Statements;
};

class BlockNode : public ASTNode
{
    GENERATED_BODY(BlockNode, ASTNode);
public:
    std::vector<ASTNode*> Statements;
};

class ErrorNode : public ASTNode
{
    GENERATED_BODY(ErrorNode, ASTNode);
};

class IdentifierNode : public ASTNode
{
    GENERATED_BODY(IdentifierNode, ASTNode);
public:
    BufferStringView Value;
    IdentifierNode(BufferStringView Value) : Value(Value) {}
};

class IntNode : public ASTNode
{
    GENERATED_BODY(IntNode, ASTNode)
public:
    int Value;
    IntNode(int Value) : Value(Value) {}
};

class FloatNode : public ASTNode
{
    GENERATED_BODY(FloatNode, ASTNode)
public:
    float Value;
    FloatNode(float Value) : Value(Value) {}
};

class BoolNode : public ASTNode
{
    GENERATED_BODY(BoolNode, ASTNode)
public:
    bool Value;
    BoolNode(bool Value) : Value(Value) {}
};

class CharNode : public ASTNode
{
    GENERATED_BODY(CharNode, ASTNode)
public:
    char Value;
    CharNode(char Value) : Value(Value) {}
};

class StringNode : public ASTNode
{
    GENERATED_BODY(StringNode, ASTNode)
public:
    BufferStringView Value;
    StringNode(BufferStringView Value) : Value(Value) {}
};

class ArrayNode : public ASTNode
{
    GENERATED_BODY(ArrayNode, ASTNode)
public:
    llvm::SmallVector<ASTNode*, 16> Elements;
    void AddItem(ASTNode* El)
    {
        Elements.push_back(El);
    }
};

class UnaryOpNode : public ASTNode
{
    GENERATED_BODY(UnaryOpNode, ASTNode)
public:
    Operator::Type Type;
    ASTNode* Operand;
    UnaryOpNode(Operator::Type Type, ASTNode* Operand)
        : Type(Type), Operand(Operand) {}
};

class PrefixOpNode : public UnaryOpNode
{
    GENERATED_BODY(PreffixOpNode, UnaryOpNode)
public:
    PrefixOpNode(Operator::Type Type, ASTNode* Operand)
        : UnaryOpNode(Type, Operand) {}
};

class SuffixOpNode : public UnaryOpNode
{
    GENERATED_BODY(SuffixOpNode, UnaryOpNode)
public:
    SuffixOpNode(Operator::Type Type, ASTNode* Operand)
        : UnaryOpNode(Type, Operand) {}
};

class BinaryOpNode : public ASTNode
{
    GENERATED_BODY(BinaryOpNode, ASTNode)
public:
    Operator::Type Type;
    ASTNode* Left;
    ASTNode* Right;
    BinaryOpNode(Operator::Type Type, ASTNode* Left, ASTNode* Right)
        : Type(Type), Left(Left), Right(Right) {}
};

class ComparisonNode : public BinaryOpNode
{
    GENERATED_BODY(EqualityNode, BinaryOpNode)
public:
    ComparisonNode(Operator::Type Type, ASTNode* Left, ASTNode* Right)
        : BinaryOpNode(Type, Left, Right) {}
};

class AssignmentNode : public BinaryOpNode
{
    GENERATED_BODY(AssignmentNode, BinaryOpNode)
public:
    AssignmentNode(Operator::Type Type, ASTNode* Left, ASTNode* Right)
        : BinaryOpNode(Type, Left, Right) {}
};

class CallNode : public ASTNode
{
    GENERATED_BODY(CallNode, ASTNode)
public:
    ASTNode* Callee;
    llvm::TinyPtrVector<ASTNode*> Arguments;
    CallNode(ASTNode* Callee) : Callee(Callee) {}
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
    SubscriptNode(ASTNode* Target, ASTNode* Index) : Target(Target), Index(Index) {}
};

class DataTypeNode : public ASTNode
{
    GENERATED_BODY(DataTypeNode, ASTNode)
public:
    DataTypeInfo TypeInfo;
    DataTypeNode(const DataTypeInfo& TypeInfo) : TypeInfo(TypeInfo) {}
};

class VariableNode : public ASTNode
{
    GENERATED_BODY(VariableNode, ASTNode)
public:
    DataTypeNode* Type;
    BufferStringView Name;
    ASTNode* Value;
    VariableNode(DataTypeNode* Type, BufferStringView Name, ASTNode* Value)
        : Type(Type), Name(Name), Value(Value) {}
};

class ParamNode : public ASTNode
{
    GENERATED_BODY(ParamNode, ASTNode)
public:
    DataTypeNode* Type;
    BufferStringView Name;
    ASTNode* DefaultValue;
    ParamNode(DataTypeNode* Type, BufferStringView Name, ASTNode* Value)
        : Type(Type), Name(Name), DefaultValue(Value) {}
};

class FunctionNode : public ASTNode
{
    GENERATED_BODY(FunctionNode, ASTNode)
public:
    DataTypeNode* ReturnType;
    BufferStringView Name;
    llvm::TinyPtrVector<ParamNode*> Params;
    ASTNode* Body = nullptr;
    FunctionNode(DataTypeNode* Type, BufferStringView Name) : ReturnType(Type), Name(Name) {}
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
    ReturnNode(ASTNode* ReturnValue) : ReturnValue(ReturnValue) {}
};

class IfNode : public ASTNode
{
    GENERATED_BODY(IfNode, ASTNode)
public:
    ASTNode* Condition;
    ASTNode* Branch;
    ASTNode* ElseBranch;
    IfNode(ASTNode* Condition, ASTNode* Branch, ASTNode* ElseBranch = nullptr)
        : Condition(Condition), Branch(Branch), ElseBranch(ElseBranch) {}
};

class WhileNode : public ASTNode
{
    GENERATED_BODY(WhileNode, ASTNode)
public:
    ASTNode* Condition;
    ASTNode* Branch;
    WhileNode(ASTNode* Condition, ASTNode* Branch) : Condition(Condition), Branch(Branch) {}
};

class ForNode : public ASTNode
{
    GENERATED_BODY(ForNode, ASTNode)
public:
    ASTNode* Initialization;
    ASTNode* Condition;
    ASTNode* Iteration;
    ASTNode* Body;
    ForNode(ASTNode* Initialization, ASTNode* Condition, ASTNode* Iteration, ASTNode* Body)
        : Initialization(Initialization), Condition(Condition), Iteration(Iteration), Body(Body) {}
};

class BreakNode : public ASTNode
{
    GENERATED_BODY(BreakNode, ASTNode)
};

class ContinueNode : public ASTNode
{
    GENERATED_BODY(ContinueNode, ASTNode)
};

#endif //CVOLT_ASTNODES_H