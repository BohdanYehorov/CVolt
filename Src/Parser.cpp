//
// Created by bohdan on 14.12.25.
//

#include "Volt/Core/Parser/Parser.h"

#include <charconv>
#include <complex>
#include <iostream>
#include <stdexcept>

namespace Volt
{
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

        std::cout << Node->GetName() <<
            " [" << Node->Pos << ":" << Node->Line << ":" << Node->Column << "] ";

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
        else if (auto Float = Cast<FloatingPointNode>(Node))
            std::cout << Float->Value << std::endl;
        else if (auto Int = Cast<IntegerNode>(Node))
            std::cout << Int->Value << std::endl;
        else if (auto Bool = Cast<BoolNode>(Node))
            std::cout << std::boolalpha << Bool->Value << std::endl;
        else if (auto Char = Cast<CharNode>(Node))
            std::cout << Char->Value << std::endl;
        else if (auto String = Cast<StringNode>(Node))
            std::cout << String->Value.ToString() << std::endl;
        else if (auto Array = Cast<ArrayNode>(Node))
        {
            std::cout << "Elements:\n";
            for (auto El : Array->Elements)
                PrintASTTree(El, Tabs + 1);
        }
        else if (auto Ref = Cast<RefNode>(Node))
        {
            std::cout << "Target:\n";
            PrintASTTree(Ref->Target, Tabs + 1);
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
        else if (auto IntType = Cast<IntegerType>(Node))
            std::cout << "Bit Width: " << IntType->BitWidth << std::endl;
        else if (auto FloatType = Cast<FloatingPointType>(Node))
            std::cout << "Bit Width: " << FloatType->BitWidth << std::endl;
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
            std::cout << "Return Value:\n";
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

    bool Parser::PrintErrors() const
    {
        for (const auto& Err : Errors)
        {
            std::cout << "ParseError: " << Err.ToString() <<
                " At position: [" << Err.Line << ":" << Err.Column << "]\n";
        }
        return HasErrors();
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
        if (!InBlock)
        {
            JumpToNextGlobalDeclaration();
            return;
        }

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            switch (Tok.Type)
            {
                case Token::OP_SEMICOLON:
                    Consume();
                    return;
                case Token::KW_LET:
                case Token::KW_IF:
                case Token::KW_WHILE:
                case Token::KW_FOR:
                case Token::KW_RETURN:
                case Token::KW_BREAK:
                case Token::KW_CONTINUE:
                case Token::OP_LBRACE:
                case Token::OP_RBRACE:
                    return;
                default:
                    break;
            }

            Consume();
        }
    }

    void Parser::JumpToNextGlobalDeclaration()
    {
        size_t BlocksCount = 0;
        while (IsValidIndex())
        {
            const Token Tok = CurrentToken();
            if (Tok.Type == Token::OP_LBRACE)
            {
                Consume();
                BlocksCount++;
                while (IsValidIndex())
                {
                    if (BlocksCount == 0)
                        break;

                    if (Peek(Token::OP_LBRACE))
                        BlocksCount++;
                    if (Peek(Token::OP_RBRACE))
                        BlocksCount--;

                    Consume();
                }

                if (BlocksCount != 0)
                    SendError(ParseErrorType::UnexpectedEOF, PrevToken().Line, PrevToken().Column);

                break;
            }

            switch (Tok.Type)
            {
                case Token::KW_FUN:
                case Token::KW_LET:
                    return;
                default:
                    Consume();
            }
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
            if (IsValidIndex())
            {
                const Token& Tok = CurrentToken();

                SendError(ParseErrorType::ExpectedToken, Tok.Line, Tok.Column,
                    { Lexer::GetOperatorLexeme(Type), GetTokenLexeme(Tok).ToString() });

                return false;
            }

            const Token& Tok = PrevToken();
            SendError(ParseErrorType::UnexpectedEOF, Tok.Line, Tok.Column);
            return false;
        }

        return true;
    }

    void Parser::SendError(ParseErrorType Type, size_t Line, size_t Column, std::vector<std::string> &&Context)
    {
        if (Errors.size() >= 100000)
            throw std::runtime_error("Error list Overload");
        Errors.emplace_back(Type, Line, Column, Context);
    }

    void Parser::SendError(ParseErrorType Type, std::vector<std::string> &&Context)
    {
        if (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            SendError(Type, Tok.Line, Tok.Column, std::move(Context));
            return;
        }

        const Token& Tok = Tokens.back();
        SendError(Type, Tok.Line, Tok.Column, std::move(Context));
    }

    bool Parser::CanBeDataType() const
    {
        if (!IsValidIndex())
            return false;

        const Token& Tok = CurrentToken();
        switch (Tok.Type)
        {
            case Token::TYPE_VOID:
            case Token::TYPE_BOOL:
            case Token::TYPE_CHAR:
            case Token::TYPE_BYTE:
            case Token::TYPE_INT:
            case Token::TYPE_LONG:
            case Token::TYPE_FLOAT:
            case Token::TYPE_DOUBLE:
                return true;
            default:
                return false;
        }
    }

    ASTNode* Parser::ParseSequence()
    {
        DepthIncScope DScope(Depth);
        auto Sequence = NodesArena.Create<SequenceNode>();

        size_t StartIndex = Index;
        while (IsValidIndex())
        {
            if (ASTNode* Expr = ParseExpression())
            {
                if (Sequence->Statements.empty())
                {
                    Sequence->Pos    = Expr->Pos;
                    Sequence->Line   = Expr->Line;
                    Sequence->Column = Expr->Column;
                }
                Sequence->Statements.push_back(Expr);
            }

            if (StartIndex == Index)
                throw std::runtime_error("Infinity loop\n");

            StartIndex = Index;
        }

        return Sequence;
    }

    ASTNode* Parser::ParseBlock()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr;
        if (!ConsumeIf(Token::OP_LBRACE, TokPtr))
        {
            while (IsValidIndex() && CurrentToken().Type != Token::OP_RBRACE)
                Consume();
            Consume();
            return nullptr;
        }

        auto Block = NodesArena.Create<BlockNode>(TokPtr->Pos, TokPtr->Line, TokPtr->Column);

        bool OldInBlock = InBlock;
        InBlock = true;
        size_t StartIndex = Index;
        while (IsValidIndex())
        {
            if (ConsumeIf(Token::OP_RBRACE))
            {
                LastNodeIsBlock = true;
                InBlock = OldInBlock;
                return Block;
            }

            if (ASTNode* Expr = ParseExpression())
            {
                Block->Statements.push_back(Expr);
                if (LastNodeIsBlock)
                    LastNodeIsBlock = false;
            }

            if (StartIndex == Index)
                throw std::runtime_error("Infinity loop");

            StartIndex = Index;
        }
        Expect(Token::OP_RBRACE);
        return nullptr;
    }

    DataTypeNode* Parser::ParseDataType()
    {
        DepthIncScope DScope(Depth);

        if (!IsValidIndex())
            return nullptr;

        const Token& Tok = CurrentToken();

        DataTypeBase* Type;
        switch (Tok.Type)
        {
            case Token::TYPE_VOID:
                Type = NodesArena.Create<VoidType>();
                break;
            case Token::TYPE_BOOL:
                Type = NodesArena.Create<BoolType>();
                break;
            case Token::TYPE_CHAR:
                Type = NodesArena.Create<CharType>();
                break;
            case Token::TYPE_BYTE:
                Type = NodesArena.Create<IntegerType>(8);
                break;
            case Token::TYPE_INT:
                Type = NodesArena.Create<IntegerType>(32);
                break;
            case Token::TYPE_LONG:
                Type = NodesArena.Create<IntegerType>(64);
                break;
            case Token::TYPE_FLOAT:
                Type = NodesArena.Create<FloatingPointType>(32);
                break;
            case Token::TYPE_DOUBLE:
                Type = NodesArena.Create<FloatingPointType>(64);
                break;
            default:
                return nullptr;
        }
        Consume();

        if (!IsValidIndex())
            return NodesArena.Create<DataTypeNode>(Type, Tok.Pos, Tok.Line, Tok.Column);

        while (true)
        {
            switch (CurrentToken().Type)
            {
                case Token::OP_MUL:
                    Type = NodesArena.Create<PointerType>(Type);
                    break;
                case Token::OP_REFERENCE:
                    Type = NodesArena.Create<ReferenceType>(Type);
                    break;
                default:
                    return NodesArena.Create<DataTypeNode>(Type, Tok.Pos, Tok.Line, Tok.Column);
            }

            Consume();
        }
    }
    ASTNode* Parser::ParseParameter()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr;
        DataTypeNode* DataType = ParseDataType();
        if (!DataType)
            return nullptr;

        if (!ConsumeIf(Token::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return nullptr;
        }

        BufferStringView Name = GetTokenLexeme(*TokPtr);

        if (!ConsumeIf(Token::OP_ASSIGN))
            return NodesArena.Create<ParamNode>(
                DataType, Name, nullptr,
                DataType->Pos,DataType->Line, DataType->Column);

        return NodesArena.Create<ParamNode>(
            DataType, Name, ParseBitwiseOR(),
            DataType->Pos, DataType->Line, DataType->Column);
    }

    ASTNode* Parser::ParseFunction()
    {
        DepthIncScope DScope(Depth);

        const Token* FirstTokPtr = nullptr;
        if (!ConsumeIf(Token::KW_FUN, FirstTokPtr))
            return nullptr;

        if (!Expect(Token::OP_COLON))
        {
            JumpToNextGlobalDeclaration();
            return nullptr;
        }

        DataTypeNode* DataType = ParseDataType();
        if (!DataType)
        {
            SendError(ParseErrorType::ExpectedDataType);
            JumpToNextGlobalDeclaration();
            return nullptr;
        }

        const Token* TokPtr;
        if (!ConsumeIf(Token::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName, TokPtr->Line, TokPtr->Column);
            JumpToNextGlobalDeclaration();
            return nullptr;
        }
        BufferStringView Name = GetTokenLexeme(*TokPtr);

        if (!Expect(Token::OP_LPAREN))
        {
            JumpToNextGlobalDeclaration();
            return nullptr;
        }

        auto Function = NodesArena.Create<FunctionNode>(DataType, Name,
            FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);

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
            JumpToNextGlobalDeclaration();
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
        DepthIncScope DScope(Depth);

        const Token* FirstTokPtr = nullptr;
        if (!ConsumeIf(Token::KW_LET, FirstTokPtr))
            return nullptr;

        if (!Expect(Token::OP_COLON))
        {
            Synchronize();
            return nullptr;
        }

        size_t StartIndex = Index;
        DataTypeNode* DataType = ParseDataType();
        if (!DataType)
        {
            SendError(ParseErrorType::ExpectedDataType);
            Synchronize();
            return nullptr;
        }

        const Token* TokPtr;
        if (!ConsumeIf(Token::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return nullptr;
        }
        BufferStringView Name = GetTokenLexeme(*TokPtr);

        if (ConsumeIf(Token::OP_ASSIGN))
        {
            ASTNode* Assign = ParseAssignment();
            if (!Assign)
            {
                SendError(ParseErrorType::ExpectedInitializerExpression);
                Synchronize();
                return nullptr;
            }

            return NodesArena.Create<VariableNode>(
               DataType, Name, Assign,
               DataType->Pos, DataType->Line, DataType->Column);
        }

        return NodesArena.Create<VariableNode>(
            DataType, Name, nullptr,
            DataType->Pos, DataType->Line, DataType->Column);
    }

    ASTNode* Parser::ParseIf()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr = nullptr;
        if (!ConsumeIf(Token::KW_IF, TokPtr))
            return nullptr;

        if (!Expect(Token::OP_LPAREN))
        {
            Synchronize();
            return nullptr;
        }

        if (ConsumeIf(Token::OP_RPAREN))
        {
            SendError(ParseErrorType::ExpectedExpression);
            Synchronize();
            return nullptr;
        }

        ASTNode* Condition = ParseAssignment();
        if (!Condition)
            return nullptr;

        if (!Expect(Token::OP_RPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Branch = nullptr;
        if (CurrentToken().Type == Token::OP_LBRACE)
            Branch = ParseBlock();
        else
        {
            Branch = ParseExpression();
            LastNodeIsBlock = true;
        }

        if (!Branch)
        {
            SendError(ParseErrorType::ExpectedStatement);
            Synchronize();
            return nullptr;
        }

        auto If = NodesArena.Create<IfNode>(
            Condition, Branch, nullptr, TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        if (!ConsumeIf(Token::KW_ELSE))
            return If;

        ASTNode* ElseBranch = nullptr;
        if (CurrentToken().Type == Token::OP_LBRACE)
            ElseBranch = ParseBlock();
        else
        {
            ElseBranch = ParseExpression();
            LastNodeIsBlock = true;
        }

        if (!ElseBranch)
        {
            SendError(ParseErrorType::ExpectedStatement);
            Synchronize();
            return nullptr;
        }

        If->ElseBranch = ElseBranch;

        return If;
    }

    ASTNode* Parser::ParseWhile()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr = nullptr;
        if (!ConsumeIf(Token::KW_WHILE, TokPtr))
            return nullptr;

        if (!Expect(Token::OP_LPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Condition = ParseAssignment();
        if (!Condition)
        {
            SendError(ParseErrorType::ExpectedExpression);
            Synchronize();
            return nullptr;
        }

        if (!Expect(Token::OP_RPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Branch = nullptr;

        bool OldInLoop = InLoop;
        InLoop = true;
        if (CurrentToken().Type == Token::OP_LBRACE)
            Branch = ParseBlock();
        else
        {
            Branch = ParseExpression();
            LastNodeIsBlock = true;
        }
        InLoop = OldInLoop;

        if (!Branch)
        {
            SendError(ParseErrorType::ExpectedStatement);
            Synchronize();
            return nullptr;
        }

        return NodesArena.Create<WhileNode>(
            Condition, Branch, TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseFor()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr = nullptr;
        if (!ConsumeIf(Token::KW_FOR, TokPtr))
            return nullptr;

        if (!Expect(Token::OP_LPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Initialization = ParseStatement();
        if (!Initialization)
        {
            SendError(ParseErrorType::ExpectedExpression);
            Synchronize();
            return nullptr;
        }

        if (!Expect(Token::OP_SEMICOLON))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Condition = ParseAssignment();
        if (!Condition)
        {
            SendError(ParseErrorType::ExpectedExpression);
            Synchronize();
            return nullptr;
        }

        if (!Expect(Token::OP_SEMICOLON))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Iteration = ParseAssignment();
        if (!Iteration)
        {
            SendError(ParseErrorType::ExpectedExpression);
            Synchronize();
            return nullptr;
        }

        if (!Expect(Token::OP_RPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Body = nullptr;
        bool OldInLoop = InLoop;
        InLoop = true;
        if (Peek(Token::OP_LBRACE))
            Body = ParseBlock();
        else
        {
            Body = ParseExpression();
            LastNodeIsBlock = true;
        }
        InLoop = OldInLoop;

        if (!Body)
        {
            SendError(ParseErrorType::ExpectedStatement);
            Synchronize();
            return nullptr;
        }

        return NodesArena.Create<ForNode>(
            Initialization, Condition, Iteration,
            Body, TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseReturn()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr = nullptr;
        if (!ConsumeIf(Token::KW_RETURN, TokPtr))
            return nullptr;

        if (!InFunction)
        {
            SendError(ParseErrorType::ReturnOutsideFunction, TokPtr->Line, TokPtr->Column);
            Synchronize();
            return nullptr;
        }

        if (Peek(Token::OP_SEMICOLON))
            return NodesArena.Create<ReturnNode>(
                nullptr, TokPtr->Pos, TokPtr->Line, TokPtr->Column);

        return NodesArena.Create<ReturnNode>(
            ParseAssignment(), TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseBreak()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr = nullptr;
        if (!ConsumeIf(Token::KW_BREAK ,TokPtr))
            return nullptr;

        if (!InLoop)
        {
            SendError(ParseErrorType::BreakOutsideLoop, TokPtr->Pos, TokPtr->Line);
            return nullptr;
        }

        return NodesArena.Create<BreakNode>(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseContinue()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr = nullptr;
        if (!ConsumeIf(Token::KW_CONTINUE, TokPtr))
            return nullptr;

        if (!InLoop)
        {
            SendError(ParseErrorType::ContinueOutsideLoop, TokPtr->Pos, TokPtr->Line);
            return nullptr;
        }

        return NodesArena.Create<ContinueNode>(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseExpression()
    {
        DepthIncScope DScope(Depth);

        ASTNode* Node = ParseStatement();
        if (!Node)
            return nullptr;

        if (LastNodeIsBlock)
        {
            SkipSemicolons();
            return Node;
        }

        Expect(Token::OP_SEMICOLON);
        SkipSemicolons();
        return Node;
    }

    ASTNode* Parser::ParseStatement()
    {
        DepthIncScope DScope(Depth);

        if (IsValidIndex() && CurrentToken().Type == Token::KW_FUN)
        {
            if (InBlock)
            {
                SendError(ParseErrorType::FunctionDefinitionNotAllowed);
                Synchronize();
                return nullptr;
            }

            return ParseFunction();
        }

        if (auto Variable = Cast<VariableNode>(ParseVariable()))
            return Variable;

        if (InBlock)
        {
            switch (CurrentToken().Type)
            {
                case Token::KW_IF:
                    return ParseIf();
                case Token::KW_WHILE:
                    return ParseWhile();
                case Token::KW_FOR:
                    return ParseFor();
                case Token::KW_RETURN:
                    return ParseReturn();
                case Token::KW_BREAK:
                    return ParseBreak();
                case Token::KW_CONTINUE:
                    return ParseContinue();
                default:
                    return ParseAssignment();
            }
        }

        SendError(ParseErrorType::ExpectedDeclaration);
        JumpToNextGlobalDeclaration();
        return nullptr;
    }

    ASTNode* Parser::ParseAssignment()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<AssignmentNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseLogicalOR()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<LogicalNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseLogicalAND()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<LogicalNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseBitwiseOR()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseBitwiseXOR()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseBitwiseAND()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseEquality()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<ComparisonNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseRelational()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<ComparisonNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseShift()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseAdditive()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseMultiplicative()
    {
        DepthIncScope DScope(Depth);

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

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseUnary()
    {
        DepthIncScope DScope(Depth);

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
                return NodesArena.Create<PrefixOpNode>(
                    OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);

            return NodesArena.Create<UnaryOpNode>(
                OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);
        }

        if (Tok.Type == Token::OP_REFERENCE)
        {
            Consume();
            ASTNode* Target = ParseUnary();
            if (!Target)
                return nullptr;

            return NodesArena.Create<RefNode>(Target, Tok.Pos, Tok.Line, Tok.Column);
        }

        return ParsePostfix();
    }

    ASTNode* Parser::ParsePostfix()
    {
        DepthIncScope DScope(Depth);

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
                    auto Call = NodesArena.Create<CallNode>(
                        Operand, Operand->Pos, Operand->Line, Operand->Column);
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

                    Operand = NodesArena.Create<SubscriptNode>(
                        Operand, Index, Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                default:
                {
                    Operator::Type OpType = Operator::GetPostfix(TokType);
                    if (OpType == Operator::UNKNOWN)
                        return Operand;
                    Consume();

                    if (OpType == Operator::INC || OpType == Operator::DEC)
                        Operand = NodesArena.Create<SuffixOpNode>(
                            OpType, Operand, Operand->Pos, Operand->Line, Operand->Column);
                    else
                        Operand = NodesArena.Create<UnaryOpNode>(
                            OpType, Operand, Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
            }
        }

        return Operand;
    }

    ASTNode* Parser::ParsePrimary()
    {
        DepthIncScope DScope(Depth);

        if (!IsValidIndex())
            return nullptr;

        const Token& Tok = CurrentToken();
        Consume();

        switch (Tok.Type)
        {
            case Token::IDENTIFIER:
                return NodesArena.Create<IdentifierNode>(
                    GetTokenLexeme(Tok), Tok.Pos, Tok.Line, Tok.Column);
            case Token::BYTE_NUMBER:
            {
                UInt8 Value;
                std::from_chars(GetTokenLexeme(Tok).CBegin(),
                    GetTokenLexeme(Tok).CEnd(), Value);
                return NodesArena.Create<IntegerNode>(
                    IntegerNode::BYTE, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case Token::INT_NUMBER:
            {
                UInt32 Value;
                std::from_chars(GetTokenLexeme(Tok).CBegin(),
                    GetTokenLexeme(Tok).CEnd(), Value);
                return NodesArena.Create<IntegerNode>(
                    IntegerNode::INT, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case Token::LONG_NUMBER:
            {
                UInt64 Value;
                std::from_chars(GetTokenLexeme(Tok).CBegin(),
                    GetTokenLexeme(Tok).CEnd(), Value);
                return NodesArena.Create<IntegerNode>(
                    IntegerNode::LONG, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case Token::FLOAT_NUMBER:
            {
                float Value;
                std::from_chars(GetTokenLexeme(Tok).CBegin(),
                    GetTokenLexeme(Tok).CEnd(), Value);
                return NodesArena.Create<FloatingPointNode>(
                    FloatingPointNode::FLOAT, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case Token::DOUBLE_NUMBER:
            {
                double Value;
                std::from_chars(GetTokenLexeme(Tok).CBegin(),
                    GetTokenLexeme(Tok).CEnd(), Value);
                return NodesArena.Create<FloatingPointNode>(
                    FloatingPointNode::DOUBLE, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case Token::BOOL_TRUE:
                return NodesArena.Create<BoolNode>(true, Tok.Pos, Tok.Line, Tok.Column);
            case Token::BOOL_FALSE:
                return NodesArena.Create<BoolNode>(false, Tok.Pos, Tok.Line, Tok.Column);
            case Token::CHAR:
                return NodesArena.Create<CharNode>(
                    GetTokenLexeme(Tok)[0], Tok.Pos, Tok.Line, Tok.Column);
            case Token::STRING:
                return NodesArena.Create<StringNode>(
                    GetTokenLexeme(Tok), Tok.Pos, Tok.Line, Tok.Column);
            case Token::OP_LPAREN:
            {
                ASTNode* Node = ParseAssignment();
                if (!Expect(Token::OP_RPAREN))
                    return nullptr;
                return Node;
            }
            case Token::OP_LBRACKET:
            {
                auto Array = NodesArena.Create<ArrayNode>(Tok.Pos, Tok.Line, Tok.Column);
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

        SendError(ParseErrorType::UnexpectedToken, Tok.Line, Tok.Column,
            { std::string(GetTokenLexeme(Tok).ToString()) });
        return nullptr;
    }
}
