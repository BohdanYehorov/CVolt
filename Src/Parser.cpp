//
// Created by bohdan on 14.12.25.
//

#include "Volt/Core/Parser/Parser.h"

#include <charconv>
#include <complex>
#include <stdexcept>

namespace Volt
{
    void Parser::WriteASTTree(std::ostream& Os, ASTNode *Node, int Tabs)
    {
        static auto PrintTabs = [&Os](int Spaces)
        {
            for (size_t i = 0; i < Spaces; i++)
                Os << "  ";
        };

        PrintTabs(Tabs);

        if (!Node)
        {
            Os << "NULL\n";
            return;
        }

        Os << Node->GetName() <<
            " [" << Node->Pos << ":" << Node->Line << ":" << Node->Column << "] ";

        if (Node->CompileTimeValue && Node->CompileTimeValue->IsValid)
        {
            Os << "CompileTimeValue: ";

            CTimeValue* Value = Node->CompileTimeValue;

            switch (DataTypeUtils::GetTypeCategory(Value->Type))
            {
                case TypeCategory::INTEGER:
                    Os << Value->Int;
                    break;
                case TypeCategory::FLOATING_POINT:
                    Os << Value->Float;
                    break;
                case TypeCategory::BOOLEAN:
                    Os << Value->Bool;
                    break;
                default:
                    Os << "Null";
                    break;
            }

            Os << " ";
        }
        if (auto Sequence = Cast<SequenceNode>(Node))
        {
            Os << std::endl;
            for (auto Statement : Sequence->Statements)
                WriteASTTree(Os, Statement, Tabs + 1);
        }
        else if (auto Block = Cast<BlockNode>(Node))
        {
            Os << std::endl;
            for (auto Statement : Block->Statements)
                WriteASTTree(Os, Statement, Tabs + 1);
        }
        else if (auto Identifier = Cast<IdentifierNode>(Node))
            Os << Identifier->Value.str() << std::endl;
        else if (auto Float = Cast<FloatingPointNode>(Node))
            Os << Float->Value << std::endl;
        else if (auto Int = Cast<IntegerNode>(Node))
            Os << Int->Value << std::endl;
        else if (auto Bool = Cast<BoolNode>(Node))
            Os << std::boolalpha << Bool->Value << std::endl;
        else if (auto Char = Cast<CharNode>(Node))
            Os << Char->Value << std::endl;
        else if (auto String = Cast<StringNode>(Node))
            Os << String->Value.str() << std::endl;
        else if (auto Array = Cast<ArrayNode>(Node))
        {
            Os << "Elements:\n";
            for (auto El : Array->Elements)
                WriteASTTree(Os, El, Tabs + 1);
        }
        else if (auto Ref = Cast<RefNode>(Node))
        {
            Os << "Target:\n";
            WriteASTTree(Os, Ref->Target, Tabs + 1);
        }
        else if (auto UnaryOp = Cast<UnaryOpNode>(Node))
        {
            Os << "OpType: " << Operator::ToString(UnaryOp->Type);
            Os << std::endl;
            WriteASTTree(Os, UnaryOp->Operand, Tabs + 1);
        }
        else if (auto BinaryOp = Cast<BinaryOpNode>(Node))
        {
            Os << "OpType: " << Operator::ToString(BinaryOp->Type);
            Os << std::endl;
            WriteASTTree(Os, BinaryOp->Left, Tabs + 1);
            WriteASTTree(Os, BinaryOp->Right, Tabs + 1);
        }
        else if (auto Call = Cast<CallNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "Callee:\n";
            WriteASTTree(Os, Call->Callee, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Args:\n";
            for (auto Arg : Call->Arguments)
                WriteASTTree(Os, Arg, Tabs + 1);
        }
        else if (auto Subscript = Cast<SubscriptNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "Target:\n";
            WriteASTTree(Os, Subscript->Target, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Index:\n";
            WriteASTTree(Os, Subscript->Index, Tabs + 1);
        }
        else if (auto IntType = Cast<IntegerType>(Node))
            Os << "Bit Width: " << IntType->BitWidth << std::endl;
        else if (auto FloatType = Cast<FloatingPointType>(Node))
            Os << "Bit Width: " << FloatType->BitWidth << std::endl;
        else if (auto Variable = Cast<VariableNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "DataType:\n";
            WriteASTTree(Os, Variable->Type, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Name: " << Variable->Name.str() << std::endl;
            PrintTabs(Tabs);
            Os << "Value:\n";
            WriteASTTree(Os, Variable->Value, Tabs + 1);
        }
        else if (auto Param = Cast<ParamNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "DataType:\n";
            WriteASTTree(Os, Param->Type, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Name: " << Param->Name.str() << std::endl;
            PrintTabs(Tabs);
            Os << "DefaultValue:\n";
            WriteASTTree(Os, Param->DefaultValue, Tabs + 1);
        }
        else if (auto Function = Cast<FunctionNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "ReturnType:\n";
            WriteASTTree(Os, Function->ReturnType, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Name: " << Function->Name.str() << std::endl;
            PrintTabs(Tabs);
            Os << "Parameters:\n";

            for (auto Parameter : Function->Params)
                WriteASTTree(Os, Parameter, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Body:\n";
            WriteASTTree(Os, Function->Body, Tabs + 1);
        }
        else if (auto Return = Cast<ReturnNode>(Node))
        {
            Os << "Return Value:\n";
            WriteASTTree(Os, Return->ReturnValue, Tabs + 1);
        }
        else if (auto If = Cast<IfNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "Condition:\n";
            WriteASTTree(Os, If->Condition, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Branch:\n";
            WriteASTTree(Os, If->Branch, Tabs + 1);
            if (If->ElseBranch)
            {
                PrintTabs(Tabs);
                Os << "ElseBranch:\n";
                WriteASTTree(Os, If->ElseBranch, Tabs + 1);
            }
        }
        else if (auto While = Cast<WhileNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "Condition:\n";
            WriteASTTree(Os, While->Condition, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Branch:\n";
            WriteASTTree(Os, While->Branch, Tabs + 1);
        }
        else if (auto For = Cast<ForNode>(Node))
        {
            Os << std::endl;
            PrintTabs(++Tabs);
            Os << "Initialization:\n";
            WriteASTTree(Os, For->Initialization, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Condition:\n";
            WriteASTTree(Os, For->Condition, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Iteration:\n";
            WriteASTTree(Os, For->Iteration, Tabs + 1);
            PrintTabs(Tabs);
            Os << "Body:\n";
            WriteASTTree(Os, For->Body, Tabs + 1);
        }
        else
            Os << std::endl;
    }

    void Parser::Parse()
    {
        Root = ParseSequence();
    }

    void Parser::WriteErrors(std::ostream &Os) const
    {
        for (const auto& Err : Errors)
        {
            Os << "ParseError: " << Err.ToString() <<
                " At position: [" << Err.Line << ":" << Err.Column << "]\n";
        }
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
            if (CurrentToken().Type != TokenType::OP_SEMICOLON)
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
                case TokenType::OP_SEMICOLON:
                    Consume();
                    return;
                case TokenType::KW_LET:
                case TokenType::KW_IF:
                case TokenType::KW_WHILE:
                case TokenType::KW_FOR:
                case TokenType::KW_RETURN:
                case TokenType::KW_BREAK:
                case TokenType::KW_CONTINUE:
                case TokenType::OP_LBRACE:
                case TokenType::OP_RBRACE:
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
            if (Tok.Type == TokenType::OP_LBRACE)
            {
                Consume();
                BlocksCount++;
                while (IsValidIndex())
                {
                    if (BlocksCount == 0)
                        break;

                    if (Peek(TokenType::OP_LBRACE))
                        BlocksCount++;
                    if (Peek(TokenType::OP_RBRACE))
                        BlocksCount--;

                    Consume();
                }

                if (BlocksCount != 0)
                    SendError(ParseErrorType::UnexpectedEOF, PrevToken().Line, PrevToken().Column);

                break;
            }

            switch (Tok.Type)
            {
                case TokenType::KW_FUN:
                case TokenType::KW_LET:
                    return;
                default:
                    Consume();
            }
        }
    }

    bool Parser::GetTokenIf(size_t Index, TokenType Type, const Token*& TokPtr) const
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

    bool Parser::GetNextTokenIf(TokenType Type, const Token *&TokPtr, size_t NextIndexOffset) const
    {
        return GetTokenIf(Index + NextIndexOffset, Type, TokPtr);
    }

    bool Parser::Peek(TokenType Type, const Token *&TokPtr) const
    {
        return GetTokenIf(Index, Type, TokPtr);
    }

    bool Parser::Peek(TokenType Type) const
    {
        if (!IsValidIndex())
            return false;
        return CurrentToken().Type == Type;
    }

    bool Parser::ConsumeIf(TokenType Type, const Token *&TokPtr)
    {
        if (GetTokenIf(Index, Type, TokPtr) && TokPtr)
        {
            Index++;
            return true;
        }

        return false;
    }

    bool Parser::ConsumeIf(TokenType Type)
    {
        if (!IsValidIndex())
            return false;

        if (CurrentToken().Type != Type)
            return false;

        Consume();
        return true;
    }

    bool Parser::Expect(TokenType Type)
    {
        if (!ConsumeIf(Type))
        {
            if (IsValidIndex())
            {
                const Token& Tok = CurrentToken();

                SendError(ParseErrorType::ExpectedToken, Tok.Line, Tok.Column,
                    { Lexer::GetOperatorLexeme(Type), GetTokenLexeme(Tok).str() });

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
        Errors.emplace_back(Type, Line, Column, std::move(Context));
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
            case TokenType::TYPE_VOID:
            case TokenType::TYPE_BOOL:
            case TokenType::TYPE_CHAR:
            case TokenType::TYPE_BYTE:
            case TokenType::TYPE_INT:
            case TokenType::TYPE_LONG:
            case TokenType::TYPE_FLOAT:
            case TokenType::TYPE_DOUBLE:
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
        if (!ConsumeIf(TokenType::OP_LBRACE, TokPtr))
        {
            while (IsValidIndex() && CurrentToken().Type != TokenType::OP_RBRACE)
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
            if (ConsumeIf(TokenType::OP_RBRACE))
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
        Expect(TokenType::OP_RBRACE);
        return nullptr;
    }

    DataTypeNodeBase* Parser::ParseDataType()
    {
        DepthIncScope DScope(Depth);

        if (!IsValidIndex())
            return nullptr;

        const Token& Tok = CurrentToken();

        PrimitiveDataType* Type;
        switch (Tok.Type)
        {
            case TokenType::TYPE_VOID:
                Type = NodesArena.Create<VoidType>();
                break;
            case TokenType::TYPE_BOOL:
                Type = NodesArena.Create<BoolType>();
                break;
            case TokenType::TYPE_CHAR:
                Type = NodesArena.Create<CharType>();
                break;
            case TokenType::TYPE_BYTE:
                Type = NodesArena.Create<IntegerType>(8);
                break;
            case TokenType::TYPE_INT:
                Type = NodesArena.Create<IntegerType>(32);
                break;
            case TokenType::TYPE_LONG:
                Type = NodesArena.Create<IntegerType>(64);
                break;
            case TokenType::TYPE_FLOAT:
                Type = NodesArena.Create<FloatingPointType>(32);
                break;
            case TokenType::TYPE_DOUBLE:
                Type = NodesArena.Create<FloatingPointType>(64);
                break;
            default:
                return nullptr;
        }
        Consume();

        DataTypeNodeBase* TypeNode = NodesArena.Create<PrimitiveTypeNode>(Type, Tok.Pos, Tok.Line, Tok.Column);

        if (!IsValidIndex())
            return NodesArena.Create<DataTypeNode>(Type, Tok.Pos, Tok.Line, Tok.Column);

        while (true)
        {
            switch (const Token& Tok = CurrentToken(); Tok.Type)
            {
                case TokenType::OP_MUL:
                    TypeNode = NodesArena.Create<PointerTypeNode>(TypeNode, Tok.Pos, Tok.Line, Tok.Column);
                    Consume();
                    break;
                case TokenType::OP_REFERENCE:
                    TypeNode = NodesArena.Create<ReferenceTypeNode>(TypeNode, Tok.Pos, Tok.Line, Tok.Column);
                    Consume();
                    break;
                case TokenType::OP_LBRACKET:
                {
                    Consume();
                    ASTNode* Length = ParseAssignment();
                    if (!Expect(TokenType::OP_RBRACKET))
                    {
                        Synchronize();
                        return nullptr;
                    }

                    TypeNode = NodesArena.Create<ArrayTypeNode>(
                        TypeNode, Length, Tok.Pos, Tok.Line, Tok.Column);
                    break;
                }
                default:
                    return TypeNode;
            }
        }
    }
    ASTNode* Parser::ParseParameter()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr;
        DataTypeNodeBase* DataType = ParseDataType();
        if (!DataType)
            return nullptr;

        if (!ConsumeIf(TokenType::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return nullptr;
        }

        llvm::StringRef Name = GetTokenLexeme(*TokPtr);

        if (!ConsumeIf(TokenType::OP_ASSIGN))
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
        if (!ConsumeIf(TokenType::KW_FUN, FirstTokPtr))
            return nullptr;

        if (!Expect(TokenType::OP_COLON))
        {
            JumpToNextGlobalDeclaration();
            return nullptr;
        }

        DataTypeNodeBase* DataType = ParseDataType();
        if (!DataType)
        {
            SendError(ParseErrorType::ExpectedDataType);
            JumpToNextGlobalDeclaration();
            return nullptr;
        }

        const Token* TokPtr;
        if (!ConsumeIf(TokenType::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName, TokPtr->Line, TokPtr->Column);
            JumpToNextGlobalDeclaration();
            return nullptr;
        }
        llvm::StringRef Name = GetTokenLexeme(*TokPtr);

        if (!Expect(TokenType::OP_LPAREN))
        {
            JumpToNextGlobalDeclaration();
            return nullptr;
        }

        auto Function = NodesArena.Create<FunctionNode>(DataType, Name,
            FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);

        while (IsValidIndex())
        {
            if (CurrentToken().Type == TokenType::OP_RPAREN)
                break;
            if (auto Parameter = Cast<ParamNode>(ParseParameter()))
                Function->AddParam(Parameter);
            else if (!ConsumeIf(TokenType::OP_COMMA))
                break;
        }

        if (!Expect(TokenType::OP_RPAREN))
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
        if (!ConsumeIf(TokenType::KW_LET, FirstTokPtr))
            return nullptr;

        if (!Expect(TokenType::OP_COLON))
        {
            Synchronize();
            return nullptr;
        }

        size_t StartIndex = Index;
        DataTypeNodeBase* DataType = ParseDataType();
        if (!DataType)
        {
            SendError(ParseErrorType::ExpectedDataType);
            Synchronize();
            return nullptr;
        }

        const Token* TokPtr;
        if (!ConsumeIf(TokenType::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return nullptr;
        }
        llvm::StringRef Name = GetTokenLexeme(*TokPtr);

        if (ConsumeIf(TokenType::OP_ASSIGN))
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
        if (!ConsumeIf(TokenType::KW_IF, TokPtr))
            return nullptr;

        if (!Expect(TokenType::OP_LPAREN))
        {
            Synchronize();
            return nullptr;
        }

        if (ConsumeIf(TokenType::OP_RPAREN))
        {
            SendError(ParseErrorType::ExpectedExpression);
            Synchronize();
            return nullptr;
        }

        ASTNode* Condition = ParseAssignment();
        if (!Condition)
            return nullptr;

        if (!Expect(TokenType::OP_RPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Branch = nullptr;
        if (CurrentToken().Type == TokenType::OP_LBRACE)
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
        if (!ConsumeIf(TokenType::KW_ELSE))
            return If;

        ASTNode* ElseBranch = nullptr;
        if (CurrentToken().Type == TokenType::OP_LBRACE)
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
        if (!ConsumeIf(TokenType::KW_WHILE, TokPtr))
            return nullptr;

        if (!Expect(TokenType::OP_LPAREN))
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

        if (!Expect(TokenType::OP_RPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Branch = nullptr;

        bool OldInLoop = InLoop;
        InLoop = true;
        if (CurrentToken().Type == TokenType::OP_LBRACE)
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
        if (!ConsumeIf(TokenType::KW_FOR, TokPtr))
            return nullptr;

        if (!Expect(TokenType::OP_LPAREN))
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

        if (!Expect(TokenType::OP_SEMICOLON))
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

        if (!Expect(TokenType::OP_SEMICOLON))
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

        if (!Expect(TokenType::OP_RPAREN))
        {
            Synchronize();
            return nullptr;
        }

        ASTNode* Body = nullptr;
        bool OldInLoop = InLoop;
        InLoop = true;
        if (Peek(TokenType::OP_LBRACE))
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
        if (!ConsumeIf(TokenType::KW_RETURN, TokPtr))
            return nullptr;

        if (!InFunction)
        {
            SendError(ParseErrorType::ReturnOutsideFunction, TokPtr->Line, TokPtr->Column);
            Synchronize();
            return nullptr;
        }

        if (Peek(TokenType::OP_SEMICOLON))
            return NodesArena.Create<ReturnNode>(
                nullptr, TokPtr->Pos, TokPtr->Line, TokPtr->Column);

        return NodesArena.Create<ReturnNode>(
            ParseAssignment(), TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseBreak()
    {
        DepthIncScope DScope(Depth);

        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_BREAK ,TokPtr))
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
        if (!ConsumeIf(TokenType::KW_CONTINUE, TokPtr))
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

        Expect(TokenType::OP_SEMICOLON);
        SkipSemicolons();
        return Node;
    }

    ASTNode* Parser::ParseStatement()
    {
        DepthIncScope DScope(Depth);

        if (IsValidIndex() && CurrentToken().Type == TokenType::KW_FUN)
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
                case TokenType::KW_IF:
                    return ParseIf();
                case TokenType::KW_WHILE:
                    return ParseWhile();
                case TokenType::KW_FOR:
                    return ParseFor();
                case TokenType::KW_RETURN:
                    return ParseReturn();
                case TokenType::KW_BREAK:
                    return ParseBreak();
                case TokenType::KW_CONTINUE:
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
            OperatorType OpType = Operator::GetAssignmentOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
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
            OperatorType OpType = Operator::GetLogicalOp(Tok.Type);
            if (OpType != OperatorType::LOGICAL_OR)
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
            OperatorType OpType = Operator::GetLogicalOp(Tok.Type);
            if (OpType != OperatorType::LOGICAL_AND)
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
            OperatorType OpType = Operator::GetBitwiseOp(Tok.Type);
            if (OpType != OperatorType::BIT_OR)
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
            OperatorType OpType = Operator::GetBitwiseOp(Tok.Type);
            if (OpType != OperatorType::BIT_XOR)
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
            OperatorType OpType = Operator::GetBitwiseOp(Tok.Type);
            if (OpType != OperatorType::BIT_AND)
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
            OperatorType OpType = Operator::GetEqualityOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
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
            OperatorType OpType = Operator::GetRelationalOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
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
            OperatorType OpType = Operator::GetShiftOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
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
            OperatorType OpType = Operator::GetAdditiveOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
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
            OperatorType OpType = Operator::GetMultiplicativeOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
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
        OperatorType OpType = Operator::GetUnaryOp(Tok.Type);
        if (OpType != OperatorType::UNKNOWN)
        {
            Consume();
            ASTNode* Operand = ParseUnary();
            if (!Operand)
                return nullptr;

            if (OpType == OperatorType::INC || OpType == OperatorType::DEC)
                return NodesArena.Create<PrefixOpNode>(
                    OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);

            return NodesArena.Create<UnaryOpNode>(
                OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);
        }

        if (Tok.Type == TokenType::OP_REFERENCE)
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
            TokenType TokType = Tok.Type;
            switch (TokType)
            {
                case TokenType::OP_LPAREN:
                {
                    auto Call = NodesArena.Create<CallNode>(
                        Operand, Operand->Pos, Operand->Line, Operand->Column);
                    Consume();
                    while (IsValidIndex())
                    {
                        if (CurrentToken().Type == TokenType::OP_RPAREN)
                            break;

                        if (ASTNode* Arg = ParseAssignment())
                            Call->AddArgument(Arg);

                        if (!ConsumeIf(TokenType::OP_COMMA))
                            break;
                    }

                    if (!Expect(TokenType::OP_RPAREN))
                        return nullptr;

                    Operand = Call;
                    break;
                }
                case TokenType::OP_LBRACKET:
                {
                    Consume();
                    ASTNode* Index = ParseAssignment();
                    if (!Index)
                        return nullptr;
                    if (!Expect(TokenType::OP_RBRACKET))
                        return nullptr;

                    Operand = NodesArena.Create<SubscriptNode>(
                        Operand, Index, Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                default:
                {
                    OperatorType OpType = Operator::GetPostfix(TokType);
                    if (OpType == OperatorType::UNKNOWN)
                        return Operand;
                    Consume();

                    if (OpType == OperatorType::INC || OpType == OperatorType::DEC)
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
            case TokenType::IDENTIFIER:
                return NodesArena.Create<IdentifierNode>(
                    GetTokenLexeme(Tok), Tok.Pos, Tok.Line, Tok.Column);
            case TokenType::BYTE_NUMBER:
            {
                UInt8 Value;
                llvm::StringRef NumStr = GetTokenLexeme(Tok);
                std::from_chars(NumStr.data(), NumStr.data() + NumStr.size(), Value);
                return NodesArena.Create<IntegerNode>(
                    IntegerNode::BYTE, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case TokenType::INT_NUMBER:
            {
                UInt32 Value;
                llvm::StringRef NumStr = GetTokenLexeme(Tok);
                std::from_chars(NumStr.data(), NumStr.data() + NumStr.size(), Value);
                return NodesArena.Create<IntegerNode>(
                    IntegerNode::INT, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case TokenType::LONG_NUMBER:
            {
                UInt64 Value;

                llvm::StringRef NumStr = GetTokenLexeme(Tok);
                std::from_chars(NumStr.data(), NumStr.data() + NumStr.size(), Value);
                return NodesArena.Create<IntegerNode>(
                    IntegerNode::LONG, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case TokenType::FLOAT_NUMBER:
            {
                float Value;

                llvm::StringRef NumStr = GetTokenLexeme(Tok);
                std::from_chars(NumStr.data(), NumStr.data() + NumStr.size(), Value);
                return NodesArena.Create<FloatingPointNode>(
                    FloatingPointNode::FLOAT, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case TokenType::DOUBLE_NUMBER:
            {
                double Value;
                llvm::StringRef NumStr = GetTokenLexeme(Tok);
                std::from_chars(NumStr.data(), NumStr.data() + NumStr.size(), Value);
                return NodesArena.Create<FloatingPointNode>(
                    FloatingPointNode::DOUBLE, Value, Tok.Pos, Tok.Line, Tok.Column);
            }
            case TokenType::BOOL_TRUE:
                return NodesArena.Create<BoolNode>(true, Tok.Pos, Tok.Line, Tok.Column);
            case TokenType::BOOL_FALSE:
                return NodesArena.Create<BoolNode>(false, Tok.Pos, Tok.Line, Tok.Column);
            case TokenType::CHAR:
                return NodesArena.Create<CharNode>(
                    GetTokenLexeme(Tok)[0], Tok.Pos, Tok.Line, Tok.Column);
            case TokenType::STRING:
                return NodesArena.Create<StringNode>(
                    GetTokenLexeme(Tok), Tok.Pos, Tok.Line, Tok.Column);
            case TokenType::OP_LPAREN:
            {
                ASTNode* Node = ParseAssignment();
                if (!Expect(TokenType::OP_RPAREN))
                    return nullptr;
                return Node;
            }
            case TokenType::OP_LBRACKET:
            {
                auto Array = NodesArena.Create<ArrayNode>(Tok.Pos, Tok.Line, Tok.Column);
                while (IsValidIndex())
                {
                    if (CurrentToken().Type == TokenType::OP_RBRACKET)
                        break;
                    ASTNode* El = ParseAssignment();
                    if (!El)
                        return nullptr;
                    Array->AddItem(El);
                    if (!ConsumeIf(TokenType::OP_COMMA))
                        break;
                }
                if (!Expect(TokenType::OP_RBRACKET))
                    return nullptr;
                return Array;
            }
            case TokenType::OP_LBRACE:
                Index--;
                return ParseBlock();
            default:
                break;
        }

        SendError(ParseErrorType::UnexpectedToken, Tok.Line, Tok.Column,
            { std::string(GetTokenLexeme(Tok).str()) });
        return nullptr;
    }
}
