//
// Created by bohdan on 14.12.25.
//

#include "Parser.h"

#include <charconv>
#include <iostream>

void Parser::PrintASTTree(ASTNode *Node, int Tabs)
{
    static auto PrintTabs = [](int Spaces)
    {
        for (size_t i = 0; i < Spaces; i++)
            std::cout << "  ";
    };

    PrintTabs(Tabs);

    if (!Node)
    {
        std::cout << "NULL\n";
        return;
    }

    std::cout << Node->GetName() << " ";

    if (auto Sequence = Cast<SequenceNode>(Node))
    {
        std::cout << std::endl;
        for (auto Statement : Sequence->Statements)
            PrintASTTree(Statement, Tabs + 1);
    }
    else if (auto Block = Cast<BlockNode>(Node))
    {
        std::cout << std::endl;
        for (auto Statement : Block->Statements)
            PrintASTTree(Statement, Tabs + 1);
    }
    else if (auto Identifier = Cast<IdentifierNode>(Node))
        std::cout << Identifier->Value.ToString() << std::endl;
    else if (auto Float = Cast<FloatNode>(Node))
        std::cout << Float->Value << std::endl;
    else if (auto Int = Cast<IntNode>(Node))
        std::cout << Int->Value << std::endl;
    else if (auto Bool = Cast<BoolNode>(Node))
        std::cout << std::boolalpha << Bool->Value << std::endl;
    else if (auto Char = Cast<CharNode>(Node))
        std::cout << Char->Value << std::endl;
    else if (auto String = Cast<StringNode>(Node))
        std::cout << String->Value.ToString() << std::endl;
    else if (auto Array = Cast<ArrayNode>(Node))
    {
        std::cout << "\bElements:\n";
        for (auto El : Array->Elements)
            PrintASTTree(El, Tabs + 1);
    }
    else if (auto UnaryOp = Cast<UnaryOpNode>(Node))
    {
        std::cout << "OpType: " << Operator::ToString(UnaryOp->Type);
        std::cout << std::endl;
        PrintASTTree(UnaryOp->Operand, Tabs + 1);
    }
    else if (auto BinaryOp = Cast<BinaryOpNode>(Node))
    {
        std::cout << "OpType: " << Operator::ToString(BinaryOp->Type);
        std::cout << std::endl;
        PrintASTTree(BinaryOp->Left, Tabs + 1);
        PrintASTTree(BinaryOp->Right, Tabs + 1);
    }
    else if (auto Call = Cast<CallNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "Callee:\n";
        PrintASTTree(Call->Callee, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Args:\n";
        for (auto Arg : Call->Arguments)
            PrintASTTree(Arg, Tabs + 1);
    }
    else if (auto Subscript = Cast<SubscriptNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "Target:\n";
        PrintASTTree(Subscript->Target, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Index:\n";
        PrintASTTree(Subscript->Index, Tabs + 1);
    }
    else if (auto Type = Cast<DataTypeNode>(Node))
    {
        std::cout << "Type: " << int(Type->TypeInfo.BaseType) << ", Size: " << Type->TypeInfo.Size << ", Align: " <<
            Type->TypeInfo.Align << std::endl;
    }
    else if (auto Variable = Cast<VariableNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "DataType:\n";
        PrintASTTree(Variable->Type, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Name: " << Variable->Name.ToString() << std::endl;
        PrintTabs(Tabs);
        std::cout << "Value:\n";
        PrintASTTree(Variable->Value, Tabs + 1);
    }
    else if (auto Param = Cast<ParamNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "DataType:\n";
        PrintASTTree(Param->Type, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Name: " << Param->Name.ToString() << std::endl;
        PrintTabs(Tabs);
        std::cout << "DefaultValue:\n";
        PrintASTTree(Param->DefaultValue, Tabs + 1);
    }
    else if (auto Function = Cast<FunctionNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "ReturnType:\n";
        PrintASTTree(Function->ReturnType, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Name: " << Function->Name.ToString() << std::endl;
        PrintTabs(Tabs);
        std::cout << "Parameters:\n";

        for (auto Parameter : Function->Params)
            PrintASTTree(Parameter, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Body:\n";
        PrintASTTree(Function->Body, Tabs + 1);
    }
    else if (auto Return = Cast<ReturnNode>(Node))
    {
        std::cout << "\b; Return Value:\n";
        PrintASTTree(Return->ReturnValue, Tabs + 1);
    }
    else if (auto If = Cast<IfNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "Condition:\n";
        PrintASTTree(If->Condition, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Branch:\n";
        PrintASTTree(If->Branch, Tabs + 1);
        if (If->ElseBranch)
        {
            PrintTabs(Tabs);
            std::cout << "ElseBranch:\n";
            PrintASTTree(If->ElseBranch, Tabs + 1);
        }
    }
    else if (auto While = Cast<WhileNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "Condition:\n";
        PrintASTTree(While->Condition, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Branch:\n";
        PrintASTTree(While->Branch, Tabs + 1);
    }
    else if (auto For = Cast<ForNode>(Node))
    {
        std::cout << std::endl;
        PrintTabs(++Tabs);
        std::cout << "Initialization:\n";
        PrintASTTree(For->Initialization, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Condition:\n";
        PrintASTTree(For->Condition, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Iteration:\n";
        PrintASTTree(For->Iteration, Tabs + 1);
        PrintTabs(Tabs);
        std::cout << "Body:\n";
        PrintASTTree(For->Body, Tabs + 1);
    }
    else
        std::cout << std::endl;
}

void Parser::Parse()
{
    Root = ParseSequence();
}

void Parser::PrintASTTree() const
{
    PrintASTTree(Root);
}

bool Parser::Consume()
{
    if (Index < Tokens.size())
    {
        Index++;
        return true;
    }
    return false;
}

void Parser::SkipSemicolons()
{
    while (IsValidIndex())
    {
        if (CurrentToken().Type != Token::OP_SEMICOLON)
            break;
        Consume();
    }
}

void Parser::Synchronize()
{
    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();

        if (Tok.Type == Token::OP_SEMICOLON)
        {
            Consume();
            return;
        }

        switch (Tok.Type)
        {
            case Token::IDENTIFIER:
            case Token::INT_NUMBER:
            case Token::FLOAT_NUMBER:
            case Token::STRING:
                return;
            default:
                break;
        }

        Consume();
    }
}

bool Parser::GetTokenIf(size_t Index, Token::TokenType Type, const Token*& TokPtr) const
{
    if (Index >= Tokens.size())
        return false;

    const Token& Tok = Tokens[Index];
    if (Tok.Type != Type)
    {
        TokPtr = nullptr;
        return false;
    }

    TokPtr = &Tok;
    return true;
}

bool Parser::GetNextTokenIf(Token::TokenType Type, const Token *&TokPtr, size_t NextIndexOffset) const
{
    return GetTokenIf(Index + NextIndexOffset, Type, TokPtr);
}

bool Parser::Peek(Token::TokenType Type, const Token *&TokPtr) const
{
    return GetTokenIf(Index, Type, TokPtr);
}

bool Parser::Peek(Token::TokenType Type) const
{
    if (!IsValidIndex())
        return false;
    return CurrentToken().Type == Type;
}

bool Parser::ConsumeIf(Token::TokenType Type, const Token *&TokPtr)
{
    if (GetTokenIf(Index, Type, TokPtr) && TokPtr)
    {
        Index++;
        return true;
    }

    return false;
}

bool Parser::ConsumeIf(Token::TokenType Type)
{
    if (!IsValidIndex())
        return false;

    if (CurrentToken().Type != Type)
        return false;

    Consume();
    return true;
}

bool Parser::Expect(Token::TokenType Type)
{
    if (!ConsumeIf(Type))
    {
        ErrorList.emplace_back(
           ErrorCode::ExpectedToken, 0, std::vector{ Lexer::GetOperatorLexeme(Type) });
        return false;
    }

    return true;
}

ASTNode* Parser::ParseSequence()
{
    auto Sequence = NodesArena.Create<SequenceNode>();

    while (IsValidIndex())
    {
        if (ASTNode* Expr = ParseExpression())
        {
            Sequence->Statements.push_back(Expr);
            if (LastNodeIsBlock)
            {
                LastNodeIsBlock = false;
                SkipSemicolons();
                continue;
            }
        }

        if (!Expect(Token::OP_SEMICOLON))
        {
            while (IsValidIndex() && !ConsumeIf(Token::OP_SEMICOLON))
                Consume();
        }
        else
            SkipSemicolons();
    }

    return Sequence;
}

ASTNode* Parser::ParseBlock()
{
    auto Block = NodesArena.Create<BlockNode>();

    if (!Expect(Token::OP_LBRACE))
    {
        while (IsValidIndex() && CurrentToken().Type != Token::OP_RBRACE)
            Consume();
        Consume();
        return nullptr;
    }

    while (IsValidIndex())
    {
        if (ConsumeIf(Token::OP_RBRACE))
        {
            LastNodeIsBlock = true;
            return Block;
        }

        if (ASTNode* Expr = ParseExpression())
        {
            Block->Statements.push_back(Expr);
            if (LastNodeIsBlock)
            {
                LastNodeIsBlock = false;
                SkipSemicolons();
                continue;
            }
        }

        if (!Expect(Token::OP_SEMICOLON))
        {
            while (IsValidIndex() && !ConsumeIf(Token::OP_SEMICOLON))
                Consume();
        }
        else
            SkipSemicolons();
    }

    Expect(Token::OP_RBRACE);
    return nullptr;
}

ASTNode* Parser::ParseDataType()
{
    if (!IsValidIndex())
        return nullptr;

    const Token& Tok = CurrentToken();

    DataType Type;
    size_t Size;
    size_t Align;
    switch (Tok.Type)
    {
        case Token::TYPE_INT:
            Type = DataType::INT;
            Size = sizeof(int);
            Align = alignof(int);
            break;
        case Token::TYPE_FLOAT:
            Type = DataType::FLOAT;
            Size = sizeof(double);
            Align = alignof(double);
            break;
        case Token::TYPE_BOOL:
            Type = DataType::BOOL;
            Size = sizeof(bool);
            Align = alignof(bool);
            break;
        case Token::TYPE_CHAR:
            Type = DataType::CHAR;
            Size = sizeof(char);
            Align = alignof(char);
            break;
        case Token::TYPE_VOID:
            Type = DataType::VOID;
            Size = 0;
            Align = 0;
            break;
        default:
            return nullptr;
    }
    Consume();

    return NodesArena.Create<DataTypeNode>(DataTypeInfo{ Type, Size, Align });
}

ASTNode* Parser::ParseParameter()
{
    const Token* TokPtr;
    auto DataType = Cast<DataTypeNode>(ParseDataType());
    if (!DataType)
        return nullptr;

    if (!ConsumeIf(Token::IDENTIFIER, TokPtr))
    {
        Synchronize();
        return nullptr;
    }

    BufferStringView Name = GetTokenLexeme(*TokPtr);

    if (!ConsumeIf(Token::OP_ASSIGN))
        return NodesArena.Create<ParamNode>(DataType, Name, nullptr);

    return NodesArena.Create<ParamNode>(DataType, Name, ParseBitwiseOR());
}

ASTNode* Parser::ParseFunction()
{
    size_t StartIndex = Index;
    const Token* TokPtr;
    auto DataType = Cast<DataTypeNode>(ParseDataType());
    if (!DataType)
        return nullptr;

    if (!ConsumeIf(Token::IDENTIFIER, TokPtr))
    {
        Index = StartIndex;
        return nullptr;
    }
    BufferStringView Name = GetTokenLexeme(*TokPtr);

    if (!ConsumeIf(Token::OP_LPAREN))
    {
        Index = StartIndex;
        return nullptr;
    }

    auto Function = NodesArena.Create<FunctionNode>(DataType, Name);
    while (IsValidIndex())
    {
        if (CurrentToken().Type == Token::OP_RPAREN)
            break;
        if (auto Parameter = Cast<ParamNode>(ParseParameter()))
            Function->AddParam(Parameter);
        else if (!ConsumeIf(Token::OP_COMMA))
            break;
    }
    if (!Expect(Token::OP_RPAREN))
    {
        Synchronize();
        return nullptr;
    }

    InFunction = true;
    Function->Body = ParseBlock();
    InFunction = false;
    if (!Function->Body)
        return nullptr;

    return Function;
}

ASTNode* Parser::ParseVariable()
{
    size_t StartIndex = Index;
    const Token* TokPtr;

    auto DataType = Cast<DataTypeNode>(ParseDataType());
    if (!DataType)
        return nullptr;

    if (!ConsumeIf(Token::IDENTIFIER, TokPtr))
    {
        Index = StartIndex;
        return nullptr;
    }
    BufferStringView Name = GetTokenLexeme(*TokPtr);

    if (ConsumeIf(Token::OP_ASSIGN))
        return NodesArena.Create<VariableNode>(DataType, Name, ParseAssignment());

    return NodesArena.Create<VariableNode>(DataType, Name, nullptr);
}

ASTNode* Parser::ParseIf()
{
    if (!ConsumeIf(Token::KW_IF))
        return nullptr;

    if (!Expect(Token::OP_LPAREN))
        return nullptr;

    ASTNode* Condition = ParseAssignment();
    if (!Condition)
        return nullptr;

    if (!Expect(Token::OP_RPAREN))
        return nullptr;

    ASTNode* Branch = nullptr;
    if (CurrentToken().Type == Token::OP_LBRACE)
        Branch = ParseBlock();
    else
    {
        Branch = ParseStatement();
        if (!LastNodeIsBlock)
            Expect(Token::OP_SEMICOLON);
        SkipSemicolons();
    }

    if (!Branch)
        return nullptr;

    auto If = NodesArena.Create<IfNode>(Condition, Branch);
    if (!ConsumeIf(Token::KW_ELSE))
        return If;

    ASTNode* ElseBranch = nullptr;
    if (CurrentToken().Type == Token::OP_LBRACE)
        ElseBranch = ParseBlock();
    else
    {
        ElseBranch = ParseStatement();
        if (!LastNodeIsBlock)
            Expect(Token::OP_SEMICOLON);
        SkipSemicolons();
    }

    if (!ElseBranch)
        return nullptr;

    If->ElseBranch = ElseBranch;

    return If;
}

ASTNode* Parser::ParseWhile()
{
    if (!ConsumeIf(Token::KW_WHILE))
        return nullptr;

    if (!Expect(Token::OP_LPAREN))
        return nullptr;

    ASTNode* Condition = ParseAssignment();
    if (!Condition)
        return nullptr;

    if (!Expect(Token::OP_RPAREN))
        return nullptr;

    ASTNode* Branch = nullptr;

    bool OldInLoop = InLoop;
    InLoop = true;
    if (CurrentToken().Type == Token::OP_LBRACE)
        Branch = ParseBlock();
    else
    {
        Branch = ParseStatement();
        if (!LastNodeIsBlock)
            Expect(Token::OP_SEMICOLON);
        SkipSemicolons();
    }
    InLoop = OldInLoop;

    if (!Branch)
        return nullptr;

    return NodesArena.Create<WhileNode>(Condition, Branch);
}

ASTNode* Parser::ParseFor()
{
    if (!ConsumeIf(Token::KW_FOR))
        return nullptr;

    if (!Expect(Token::OP_LPAREN))
        return nullptr;

    ASTNode* Initialization = ParseStatement();
    if (!Expect(Token::OP_SEMICOLON))
        return nullptr;

    ASTNode* Condition = ParseAssignment();
    if (!Expect(Token::OP_SEMICOLON))
        return nullptr;

    ASTNode* Iteration = ParseAssignment();
    if (!Expect(Token::OP_RPAREN))
        return nullptr;

    ASTNode* Body = nullptr;
    LoopsCount++;
    if (Peek(Token::OP_LBRACE))
        Body = ParseBlock();
    else
    {
        Body = ParseStatement();
        if (!LastNodeIsBlock)
            Expect(Token::OP_SEMICOLON);
        SkipSemicolons();
    }
    LoopsCount--;

    if (!Body)
        return nullptr;

    return NodesArena.Create<ForNode>(Initialization, Condition, Iteration, Body);
}

ASTNode* Parser::ParseReturn()
{
    if (!InFunction)
        return nullptr;

    if (!ConsumeIf(Token::KW_RETURN))
        return nullptr;

    if (Peek(Token::OP_SEMICOLON))
        return NodesArena.Create<ReturnNode>(nullptr);

    return NodesArena.Create<ReturnNode>(ParseAssignment());
}

ASTNode* Parser::ParseBreak()
{
    if (LoopsCount == 0)
        return nullptr;

    if (!ConsumeIf(Token::KW_BREAK))
        return nullptr;

    return NodesArena.Create<BreakNode>();
}

ASTNode* Parser::ParseContinue()
{
    if (LoopsCount == 0)
        return nullptr;

    if (!ConsumeIf(Token::KW_CONTINUE))
        return nullptr;

    return NodesArena.Create<ContinueNode>();
}

ASTNode* Parser::ParseExpression()
{
    return ParseStatement();
}

ASTNode* Parser::ParseStatement()
{
    if (auto Function = Cast<FunctionNode>(ParseFunction()))
        return Function;
    if (auto Variable = Cast<VariableNode>(ParseVariable()))
        return Variable;
    if (auto If = Cast<IfNode>(ParseIf()))
        return If;
    if (auto While = Cast<WhileNode>(ParseWhile()))
        return While;
    if (auto For = Cast<ForNode>(ParseFor()))
        return For;
    if (auto Return = Cast<ReturnNode>(ParseReturn()))
        return Return;
    if (auto Break = Cast<BreakNode>(ParseBreak()))
        return Break;
    if (auto Continue = Cast<ContinueNode>(ParseContinue()))
        return Continue;

    return ParseAssignment();
}

ASTNode* Parser::ParseAssignment()
{
    ASTNode* Left = ParseLogicalOR();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetAssignmentOp(Tok.Type);
        if (OpType == Operator::UNKNOWN)
            break;
        Consume();
        ASTNode* Right = ParseAssignment();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<AssignmentNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseLogicalOR()
{
    ASTNode* Left = ParseLogicalAND();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetLogicalOp(Tok.Type);
        if (OpType != Operator::LOGICAL_OR)
            break;
        Consume();
        ASTNode* Right = ParseLogicalAND();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseLogicalAND()
{
    ASTNode* Left = ParseBitwiseOR();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetLogicalOp(Tok.Type);
        if (OpType != Operator::LOGICAL_AND)
            break;
        Consume();
        ASTNode* Right = ParseBitwiseOR();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseBitwiseOR()
{
    ASTNode* Left = ParseBitwiseXOR();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetBitwiseOp(Tok.Type);
        if (OpType != Operator::BIT_OR)
            break;
        Consume();
        ASTNode* Right = ParseBitwiseXOR();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseBitwiseXOR()
{
    ASTNode* Left = ParseBitwiseAND();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetBitwiseOp(Tok.Type);
        if (OpType != Operator::BIT_XOR)
            break;
        Consume();
        ASTNode* Right = ParseBitwiseAND();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseBitwiseAND()
{
    ASTNode* Left = ParseEquality();
    if (!Left)
        return nullptr;
    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetBitwiseOp(Tok.Type);
        if (OpType != Operator::BIT_AND)
            break;
        Consume();
        ASTNode* Right = ParseEquality();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseEquality()
{
    ASTNode* Left = ParseRelational();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetEqualityOp(Tok.Type);
        if (OpType == Operator::UNKNOWN)
            break;
        Consume();
        ASTNode* Right = ParseRelational();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<ComparisonNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseRelational()
{
    ASTNode* Left = ParseShift();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetRelationalOp(Tok.Type);
        if (OpType == Operator::UNKNOWN)
            break;
        Consume();
        ASTNode* Right = ParseShift();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<ComparisonNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseShift()
{
    ASTNode* Left = ParseAdditive();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetShiftOp(Tok.Type);
        if (OpType == Operator::UNKNOWN)
            break;
        Consume();
        ASTNode* Right = ParseAdditive();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseAdditive()
{
    ASTNode* Left = ParseMultiplicative();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetAdditiveOp(Tok.Type);
        if (OpType == Operator::UNKNOWN)
            break;
        Consume();
        ASTNode* Right = ParseMultiplicative();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseMultiplicative()
{
    ASTNode* Left = ParseUnary();
    if (!Left)
        return nullptr;

    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Operator::Type OpType = Operator::GetMultiplicativeOp(Tok.Type);
        if (OpType == Operator::UNKNOWN)
            break;
        Consume();
        ASTNode* Right = ParseUnary();
        if (!Right)
            return nullptr;

        Left = NodesArena.Create<BinaryOpNode>(OpType, Left, Right);
    }

    return Left;
}

ASTNode* Parser::ParseUnary()
{
    if (!IsValidIndex())
        return nullptr;

    const Token& Tok = CurrentToken();
    Operator::Type OpType = Operator::GetUnaryOp(Tok.Type);
    if (OpType != Operator::UNKNOWN)
    {
        Consume();
        ASTNode* Operand = ParseUnary();
        if (!Operand)
            return nullptr;

        if (OpType == Operator::INC || OpType == Operator::DEC)
            return NodesArena.Create<PrefixOpNode>(OpType, Operand);

        return NodesArena.Create<UnaryOpNode>(OpType, Operand);
    }
    return ParsePostfix();
}

ASTNode* Parser::ParsePostfix()
{
    ASTNode* Operand = ParsePrimary();
    if (!Operand) return nullptr;
    while (IsValidIndex())
    {
        const Token& Tok = CurrentToken();
        Token::TokenType TokType = Tok.Type;
        switch (TokType)
        {
            case Token::OP_LPAREN:
            {
                auto Call = NodesArena.Create<CallNode>(Operand);
                Consume();
                while (IsValidIndex())
                {
                    if (CurrentToken().Type == Token::OP_RPAREN)
                        break;

                    if (ASTNode* Arg = ParseAssignment())
                        Call->AddArgument(Arg);

                    if (!ConsumeIf(Token::OP_COMMA))
                        break;
                }

                if (!Expect(Token::OP_RPAREN))
                    return nullptr;

                Operand = Call;
                break;
            }
            case Token::OP_LBRACKET:
            {
                Consume();
                ASTNode* Index = ParseAssignment();
                if (!Index)
                    return nullptr;
                if (!Expect(Token::OP_RBRACKET))
                    return nullptr;

                Operand = NodesArena.Create<SubscriptNode>(Operand, Index);
            }
            default:
            {
                Operator::Type OpType = Operator::GetPostfix(TokType);
                if (OpType == Operator::UNKNOWN)
                    return Operand;
                Consume();

                if (OpType == Operator::INC || OpType == Operator::DEC)
                    Operand = NodesArena.Create<SuffixOpNode>(OpType, Operand);
                else
                    Operand = NodesArena.Create<UnaryOpNode>(OpType, Operand);

                break;
            }
        }
    }

    return Operand;
}

ASTNode* Parser::ParsePrimary()
{
    if (!IsValidIndex())
        return nullptr;

    const Token& Tok = CurrentToken();
    Consume();

    switch (Tok.Type)
    {
        case Token::IDENTIFIER:
            return NodesArena.Create<IdentifierNode>(GetTokenLexeme(Tok));
        case Token::FLOAT_NUMBER:
        {
            float Value;
            std::from_chars(GetTokenLexeme(Tok).CBegin(), GetTokenLexeme(Tok).CEnd(), Value);
            return NodesArena.Create<FloatNode>(Value);
        }
        case Token::INT_NUMBER:
        {
            int Value;
            std::from_chars(GetTokenLexeme(Tok).CBegin(), GetTokenLexeme(Tok).CEnd(), Value);
            return NodesArena.Create<IntNode>(Value);
        }
        case Token::BOOL_TRUE:
            return NodesArena.Create<BoolNode>(true);
        case Token::BOOL_FALSE:
            return NodesArena.Create<BoolNode>(false);
        case Token::CHAR:
            return NodesArena.Create<CharNode>(GetTokenLexeme(Tok)[0]);
        case Token::STRING:
            return NodesArena.Create<StringNode>(GetTokenLexeme(Tok));
        case Token::OP_LPAREN:
        {
            ASTNode* Node = ParseExpression();
            if (!Expect(Token::OP_RPAREN))
                return nullptr;
            return Node;
        }
        case Token::OP_LBRACKET:
        {
            auto Array = NodesArena.Create<ArrayNode>();
            while (IsValidIndex())
            {
                if (CurrentToken().Type == Token::OP_RBRACKET)
                    break;
                ASTNode* El = ParseAssignment();
                if (!El)
                    return nullptr;
                Array->AddItem(El);
                if (!ConsumeIf(Token::OP_COMMA))
                    break;
            }
            if (!Expect(Token::OP_RBRACKET))
                return nullptr;
            return Array;
        }
        case Token::OP_LBRACE:
            Index--;
            return ParseBlock();
        default:
            break;
    }

    ErrorList.emplace_back(ErrorCode::UnexpectedToken, Tok.Pos, std::vector{ std::string(GetTokenLexeme(Tok).ToString()) });
    return nullptr;
}