//
// Created by bohdan on 8/19/26.
//

#ifndef CVOLT_NODEKIND_H
#define CVOLT_NODEKIND_H

namespace Volt
{
    enum class NodeKind
    {
        ASTNode,
        SequenceNode,
        BlockNode,
        IdentifierNode,
        IntegerNode,
        FloatingPointNode,
        BoolNode,
        CharNode,
        StringNode,
        ArrayNode,
        NullPointerNode,
        RefNode,
        UnrefNode,
        UnaryOpNode,
        PrefixOpNode,
        SuffixOpNode,
        BinaryOpNode,
        ComparisonNode,
        LogicalNode,
        AssignmentNode,
        CallNode,
        SubscriptNode,
        SizeOfNode,
        AlignOfNode,
        DataTypeNodeBase,
        QualTypeNode,
        PrimitiveTypeNode,
        DerivedTypeNode,
        PointerTypeNode,
        ReferenceTypeNode,
        ArrayTypeNode,
        ClassTypeNode,
        TypeOfNode,
        ConstructNode,
        ExplicitCastNode,
        VariableNodeBase,
        VariableNode,
        VariableConstructNode,
        ParamNode,
        FunctionNodeBase,
        FunctionNode,
        ConstructorNode,
        ReturnNode,
        FieldNode,
        ClassNode,
        MemberAccessNode,
        IfNode,
        WhileNode,
        ForNode,
        BreakNode,
        ContinueNode,
        ErrorNode
    };
}

#endif //CVOLT_NODEKIND_H
