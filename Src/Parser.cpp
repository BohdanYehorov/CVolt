//
// Created by bohdan on 14.12.25.
//

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Core/TypeChecker/ExprResult.h"

#include <charconv>
#include <complex>

namespace Volt
{
    void Parser::Parse()
    {
        if (CContext.HasErrors()) return;
        Root = ParseSequence();
    }

    bool Parser::Consume()
    {
        if (Index < Tokens.Length())
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

    void Parser::SkipExpressionInBrackets(TokenType OpenBracket, TokenType CloseBracket, int ConsumedBrackets)
    {
        int BracketsCount = ConsumedBrackets;
        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            if (Tok.Type == OpenBracket)
                BracketsCount++;
            else if (Tok.Type == CloseBracket)
                BracketsCount--;

            Consume();
            if (BracketsCount <= 0)
                break;
        }
        if (BracketsCount != 0)
            SendError(ParseErrorType::UnexpectedEOF);
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
                case TokenType::KW_CLASS:
                    return;
                default:
                    Consume();
            }
        }
    }

    bool Parser::GetTokenIf(size_t Index, TokenType Type, const Token*& TokPtr) const
    {
        if (Index >= Tokens.Length())
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

    bool Parser::ExpectAndConsume(TokenType Type)
    {
        if (Expect(Type)) return true;
        Consume();
        return false;
    }

    void Parser::SendError(ParseErrorType Type, size_t Line, size_t Column, Array<std::string> &&Context)
    {
        if (Errors.Length() >= 100000)
            VoltUnreachable("Error list Overload");
        Errors.Emplace(Type, Line, Column, std::move(Context));
    }

    void Parser::SendError(ParseErrorType Type, Array<std::string> &&Context)
    {
        if (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            SendError(Type, Tok.Line, Tok.Column, std::move(Context));
            return;
        }

        const Token& Tok = Tokens.Back();
        SendError(Type, Tok.Line, Tok.Column, std::move(Context));
    }

    ASTNode* Parser::ParseSequence()
    {
        auto Sequence = NodesArena.Create<SequenceNode>();

        size_t StartIndex = Index;
        while (IsValidIndex())
        {
            if (ASTNode* Expr = ParseExpression())
            {
                if (Sequence->Statements.Empty())
                {
                    Sequence->Pos    = Expr->Pos;
                    Sequence->Line   = Expr->Line;
                    Sequence->Column = Expr->Column;
                }
                Sequence->Statements.Add(Expr);
            }

            if (StartIndex == Index)
                VoltUnreachable("Infinity loop");

            StartIndex = Index;
        }

        return Sequence;
    }

    ASTNode* Parser::ParseBlock()
    {
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
                Block->Statements.Add(Expr);
                if (LastNodeIsBlock)
                    LastNodeIsBlock = false;
            }

            if (StartIndex == Index)
                VoltUnreachable("Infinity loop");

            StartIndex = Index;
        }
        Expect(TokenType::OP_RBRACE);
        return nullptr;
    }

    ASTNode *Parser::ParseReferenceType()
    {
        ASTNode* TypeNode = ParseQualType();
        if (!IsValidNode(TypeNode))
            return TypeNode;

        if (ConsumeIf(TokenType::OP_REFERENCE))
            return NodesArena.Create<ReferenceTypeNode>(
                Cast<DataTypeNodeBase>(TypeNode), TypeNode->Pos, TypeNode->Line, TypeNode->Column);

        return TypeNode;
    }

    ASTNode *Parser::ParseQualType()
    {
        if (!IsValidIndex())
            return nullptr;

        const Token* TokPtr;
        if (ConsumeIf(TokenType::TYPE_CONST, TokPtr))
        {
            ASTNode* WrappedTypeNode = ParseWrappedType();
            if (!IsValidNode(WrappedTypeNode))
                return WrappedTypeNode;

            return NodesArena.Create<QualTypeNode>(
               Cast<DataTypeNodeBase>(WrappedTypeNode), QualType::CONST, TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        }

        return ParseWrappedType();
    }

    ASTNode *Parser::ParseWrappedType()
    {
        ASTNode* TypeNode = ParsePrimitiveType();
        if (!IsValidNode(TypeNode))
            return TypeNode;

        while (IsValidIndex())
        {
            switch (const Token& Tok = CurrentToken(); Tok.Type)
            {
                case TokenType::OP_MUL:
                    TypeNode = NodesArena.Create<PointerTypeNode>(
                        Cast<DataTypeNodeBase>(TypeNode), Tok.Pos, Tok.Line, Tok.Column);
                    Consume();
                    break;
                case TokenType::OP_LBRACKET:
                {
                    Consume();
                    ASTNode* Length = ParseAssignment();
                    if (!Expect(TokenType::OP_RBRACKET))
                    {
                        Synchronize();
                        return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
                    }

                    TypeNode = NodesArena.Create<ArrayTypeNode>(
                        Cast<DataTypeNodeBase>(TypeNode), Length, Tok.Pos, Tok.Line, Tok.Column);
                    break;
                }
                default:
                    return TypeNode;
            }
        }

        return TypeNode;
    }

    ASTNode *Parser::ParsePrimitiveType()
    {
        using enum TokenType;

        const Token& Tok = CurrentToken();

        PrimitiveDataType* Type;
        switch (Tok.Type)
        {
            case TYPE_VOID: Type = CContext.GetVoidType(); break;
            case TYPE_BOOL: Type = CContext.GetBoolType(); break;
            case TYPE_CHAR: Type = CContext.GetCharType(); break;
            case TYPE_I8:   Type = CContext.GetIntegerType(8, true);   break;
            case TYPE_I16:  Type = CContext.GetIntegerType(16, true);  break;
            case TYPE_I32:  Type = CContext.GetIntegerType(32, true);  break;
            case TYPE_I64:  Type = CContext.GetIntegerType(64, true);  break;
            case TYPE_U8:   Type = CContext.GetIntegerType(8, false);  break;
            case TYPE_U16:  Type = CContext.GetIntegerType(16, false); break;
            case TYPE_U32:  Type = CContext.GetIntegerType(32, false); break;
            case TYPE_U64:  Type = CContext.GetIntegerType(64, false); break;
            case TYPE_F16:  Type = CContext.GetFPType(16);  break;
            case TYPE_F32:  Type = CContext.GetFPType(32);  break;
            case TYPE_F64:  Type = CContext.GetFPType(64);  break;
            case TYPE_F128: Type = CContext.GetFPType(128); break;
            case IDENTIFIER:
            {
                llvm::StringRef Lexeme = GetTokenLexeme(Tok);
                if (!CustomTypes.contains(Lexeme) && !CContext.GetClassType(Lexeme.str())) return nullptr;
                Consume();
                return NodesArena.Create<ClassTypeNode>(Lexeme, Tok.Pos, Tok.Line, Tok.Column);
            }
            case OP_LPAREN:
            {
                Consume();
                ASTNode* Node = ParseQualType();
                Expect(OP_RPAREN);
                return Node;
            }
            default:
                return nullptr;
        }
        Consume();

        return NodesArena.Create<PrimitiveTypeNode>(
            Type, Tok.Pos, Tok.Line, Tok.Column);
    }

    ASTNode* Parser::ParseParameter()
    {
        const Token* TokPtr;
        ASTNode* DataType = ParseDataType();
        if (!IsValidNode(DataType))
            return DataType;

        if (!ConsumeIf(TokenType::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            return CreateErrorNodeOnCurrentOrEndToken();
        }

        llvm::StringRef Name = GetTokenLexeme(*TokPtr);

        DataTypeNodeBase* DataTypeNode = Cast<DataTypeNodeBase>(DataType);
        if (!ConsumeIf(TokenType::OP_ASSIGN))
            return NodesArena.Create<ParamNode>(
                DataTypeNode, Name, nullptr,
                DataTypeNode->Pos,DataTypeNode->Line, DataTypeNode->Column);

        return NodesArena.Create<ParamNode>(
            DataTypeNode, Name, ParseBitwiseOR(),
            DataTypeNode->Pos, DataTypeNode->Line, DataTypeNode->Column);
    }

    ASTNode* Parser::ParseFunction()
    {
        const Token* FirstTokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_FUN, FirstTokPtr))
            return nullptr;

        Expect(TokenType::OP_COLON);

        ASTNode* DataType = ParseDataType();
        if (IsErrorNode(DataType))
        {
            JumpToNextGlobalDeclaration();
            return DataType;
        }

        if (!DataType)
        {
            SendError(ParseErrorType::ExpectedDataType);
            JumpToNextGlobalDeclaration();
            return CreateErrorNodeOnCurrentOrEndToken();
        }

        bool CanBuildNode = true;
        const Token* TokPtr;
        if (!ConsumeIf(TokenType::IDENTIFIER, TokPtr))
        {
            const Token& Tok = CurrentOrEndToken();
            SendError(ParseErrorType::ExpectedDeclaratorName, Tok.Line, Tok.Column);
            CanBuildNode = false;
        }

        ArgsVector<ParamNode*> Params;
        ParseParametersInParens(Params);

        InFunction = true;
        ASTNode* Body = ParseBlock();
        InFunction = false;
        if (!Body)
        {
            SendError(ParseErrorType::ExpectedFunctionBody);
            return CreateErrorNodeOnCurrentOrEndToken();
        }

        if (IsErrorNode(Body))
            return Body;

        if (!CanBuildNode)
            return CreateErrorNode(FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);

        return NodesArena.Create<FunctionNode>(Cast<DataTypeNodeBase>(DataType), GetTokenLexeme(*TokPtr),
            std::move(Params), Cast<BlockNode>(Body), FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
    }

    ASTNode* Parser::ParseVariable()
    {
        const Token* FirstTokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_LET, FirstTokPtr))
            return nullptr;

        Expect(TokenType::OP_COLON);

        ASTNode* DataType = ParseDataType();
        if (IsErrorNode(DataType))
        {
            JumpToOneOfTokens(TokenType::IDENTIFIER);
        }

        if (!DataType)
        {
            SendError(ParseErrorType::ExpectedDataType);
            Synchronize();
            return CreateErrorNode(FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
        }

        const Token* TokPtr;
        if (!ConsumeIf(TokenType::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return CreateErrorNode(FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
        }
        llvm::StringRef Name = GetTokenLexeme(*TokPtr);

        DataTypeNodeBase* DataTypeNode = Cast<DataTypeNodeBase>(DataType);
        if (ConsumeIf(TokenType::OP_ASSIGN))
        {
            ASTNode* Assign = ParseAssignment();
            if (!Assign)
            {
                SendError(ParseErrorType::ExpectedInitializerExpression);
                Synchronize();
                return CreateErrorNodeOnCurrentOrEndToken();
            }

            return NodesArena.Create<VariableNode>(
               DataTypeNode, Name, Assign,
               FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
        }

        if (ConsumeIf(TokenType::OP_LPAREN))
        {
            ArgsVector<ASTNode*> Args;
            ParseExpressionsSeparatedByComma(Args, TokenType::OP_RPAREN);
            Consume();

            return NodesArena.Create<VariableConstructNode>(
                DataTypeNode, Name, std::move(Args),
                FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
        }

        return NodesArena.Create<VariableNode>(
            DataTypeNode, Name, nullptr,
            FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
    }

    ASTNode* Parser::ParseClass()
    {
        const Token* FirstTokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_CLASS, FirstTokPtr))
            return nullptr;

        const Token* TokPtr;
        if (!ConsumeIf(TokenType::IDENTIFIER, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return nullptr;
        }
        llvm::StringRef Name = GetTokenLexeme(*TokPtr);
        CustomTypes.insert(Name);

        Expect(TokenType::OP_LBRACE);

        SmallVec4<VariableNode*> Fields;
        SmallVec4<FunctionNode*> Methods;
        SmallVec4<ConstructorNode*> Constructors;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            TokenType TokType = Tok.Type;
            if (TokType == TokenType::OP_RBRACE)
                break;

            switch (TokType)
            {
                case TokenType::KW_LET:
                {
                    if (VariableNode* Field = Cast<VariableNode>(ParseVariable()))
                    {
                        Fields.push_back(Field);
                        Expect(TokenType::OP_SEMICOLON);
                        SkipSemicolons();
                        break;
                    }
                    return nullptr;
                }
                case TokenType::KW_FUN:
                {
                    if (FunctionNode* Method = Cast<FunctionNode>(ParseFunction()))
                    {
                        Methods.push_back(Method);
                        break;
                    }
                    return nullptr;
                }
                case TokenType::IDENTIFIER:
                {
                    if (GetTokenLexeme(Tok) != Name)
                    {
                        SendError(ParseErrorType::ExpectedDeclaration);
                        Synchronize();
                        return CreateErrorNodeOnCurrentOrEndToken();
                    }

                    Consume();

                    ArgsVector<ParamNode*> Params;
                    ParseParametersInParens(Params);

                    InFunction = true;
                    ASTNode* Body = ParseBlock();
                    InFunction = false;
                    if (!Body)
                    {
                        SendError(ParseErrorType::ExpectedFunctionBody);
                        return CreateErrorNodeOnCurrentOrEndToken();
                    }

                    Constructors.push_back(NodesArena.Create<ConstructorNode>(std::move(Params),
                                           Cast<BlockNode>(Body), Tok.Pos, Tok.Line, Tok.Column));
                    break;
                }
                default:
                    SendError(ParseErrorType::ExpectedDeclaration);
                    Synchronize();
                    return CreateErrorNodeOnCurrentOrEndToken();
            }
        }

        Expect(TokenType::OP_RBRACE);

        return NodesArena.Create<ClassNode>(Name, std::move(Fields), std::move(Methods),
                                            std::move(Constructors), FirstTokPtr->Pos,
                                            FirstTokPtr->Line, FirstTokPtr->Column);
    }

    ASTNode* Parser::ParseIf()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_IF, TokPtr))
            return nullptr;

        Expect(TokenType::OP_LPAREN);

        ASTNode* Condition = nullptr;
        if (ConsumeIf(TokenType::OP_RPAREN))
            SendError(ParseErrorType::ExpectedExpression);
        else
            Condition = ParseAssignment();

        Expect(TokenType::OP_RPAREN);

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
            return CreateErrorNode(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        }

        If->ElseBranch = ElseBranch;

        return If;
    }

    ASTNode* Parser::ParseWhile()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_WHILE, TokPtr))
            return nullptr;

        Expect(TokenType::OP_LPAREN);

        ASTNode* Condition = nullptr;
        if (ConsumeIf(TokenType::OP_RPAREN))
            SendError(ParseErrorType::ExpectedExpression);
        else
            Condition = ParseAssignment();

        Expect(TokenType::OP_RPAREN);

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

        if (IsErrorNode(Branch))
            return Branch;

        if (!Branch)
        {
            SendError(ParseErrorType::ExpectedStatement);
            Synchronize();
            return CreateErrorNode(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        }

        return NodesArena.Create<WhileNode>(
            Condition, Branch, TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseFor()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_FOR, TokPtr))
            return nullptr;

        Expect(TokenType::OP_LPAREN);

        ASTNode* Initialization = ParseForInitialization();
        ASTNode* Condition =      ParseForCondition();
        ASTNode* Iteration =      ParseFotIteration();

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

        if (IsErrorNode(Body))
            return Body;

        if (!Body)
        {
            SendError(ParseErrorType::ExpectedStatement);
            Synchronize();
            return CreateErrorNode(0, 0, 0);
        }

        return NodesArena.Create<ForNode>(
            Initialization, Condition, Iteration,
            Body, TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode *Parser::ParseForInitialization()
    {
        if (ConsumeIf(TokenType::OP_SEMICOLON))
            return nullptr;

        ASTNode* Initialization = ParseStatement();
        if (!Initialization)
            SendError(ParseErrorType::ExpectedInitializerExpression);

        Expect(TokenType::OP_SEMICOLON);
        return Initialization;
    }

    ASTNode *Parser::ParseForCondition()
    {
        if (ConsumeIf(TokenType::OP_SEMICOLON))
            return nullptr;

        ASTNode* Condition = ParseAssignment();
        if (!Condition)
            SendError(ParseErrorType::ExpectedExpression);

        Expect(TokenType::OP_SEMICOLON);
        return Condition;
    }

    ASTNode *Parser::ParseFotIteration()
    {
        if (ConsumeIf(TokenType::OP_RPAREN))
            return nullptr;

        ASTNode* Iteration = ParseAssignment();
        if (!Iteration)
            SendError(ParseErrorType::ExpectedExpression);

        Expect(TokenType::OP_RPAREN);
        return Iteration;
    }

    ASTNode* Parser::ParseReturn()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_RETURN, TokPtr))
            return nullptr;

        if (!InFunction)
        {
            SendError(ParseErrorType::ReturnOutsideFunction, TokPtr->Line, TokPtr->Column);
            Synchronize();
            return CreateErrorNode(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        }

        if (Peek(TokenType::OP_SEMICOLON))
            return NodesArena.Create<ReturnNode>(
                nullptr, TokPtr->Pos, TokPtr->Line, TokPtr->Column);

        return NodesArena.Create<ReturnNode>(
            ParseAssignment(), TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseBreak()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_BREAK ,TokPtr))
            return nullptr;

        if (!InLoop)
        {
            SendError(ParseErrorType::BreakOutsideLoop, TokPtr->Pos, TokPtr->Line);
            return CreateErrorNode(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        }

        return NodesArena.Create<BreakNode>(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseContinue()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KW_CONTINUE, TokPtr))
            return nullptr;

        if (!InLoop)
        {
            SendError(ParseErrorType::ContinueOutsideLoop, TokPtr->Pos, TokPtr->Line);
            return CreateErrorNode(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        }

        return NodesArena.Create<ContinueNode>(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseExpression()
    {
        ASTNode* Node = ParseStatement();
        if (!IsValidNode(Node))
        {
            SkipSemicolons();
            return Node;
        }

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
        if (!IsValidIndex())
            return nullptr;

        const Token& Tok = CurrentToken();
        if (Tok.Type == TokenType::KW_FUN)
        {
            if (InBlock)
            {
                SendError(ParseErrorType::FunctionDefinitionNotAllowed);
                Synchronize();
                return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
            }

            return ParseFunction();
        }

        if (Tok.Type == TokenType::KW_CLASS)
        {
            if (InBlock)
            {
                // SendError
                Synchronize();
                return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
            }

            return ParseClass();
        }

        if (auto Variable = ParseVariable())
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
        return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
    }

    ASTNode* Parser::ParseAssignment()
    {
        ASTNode* Left = ParseLogicalOR();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetAssignmentOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
                break;
            Consume();
            ASTNode* Right = ParseAssignment();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<AssignmentNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseLogicalOR()
    {
        ASTNode* Left = ParseLogicalAND();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetLogicalOp(Tok.Type);
            if (OpType != OperatorType::LOGICAL_OR)
                break;
            Consume();
            ASTNode* Right = ParseLogicalAND();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<LogicalNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseLogicalAND()
    {
        ASTNode* Left = ParseBitwiseOR();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetLogicalOp(Tok.Type);
            if (OpType != OperatorType::LOGICAL_AND)
                break;
            Consume();
            ASTNode* Right = ParseBitwiseOR();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<LogicalNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseBitwiseOR()
    {
        ASTNode* Left = ParseBitwiseXOR();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetBitwiseOp(Tok.Type);
            if (OpType != OperatorType::BIT_OR)
                break;
            Consume();
            ASTNode* Right = ParseBitwiseXOR();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
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
            OperatorType OpType = Operator::GetBitwiseOp(Tok.Type);
            if (OpType != OperatorType::BIT_XOR)
                break;
            Consume();
            ASTNode* Right = ParseBitwiseAND();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseBitwiseAND()
    {
        ASTNode* Left = ParseEquality();
        if (!IsValidNode(Left))
            return Left;
        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetBitwiseOp(Tok.Type);
            if (OpType != OperatorType::BIT_AND)
                break;
            Consume();
            ASTNode* Right = ParseEquality();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseEquality()
    {
        ASTNode* Left = ParseRelational();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetEqualityOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
                break;
            Consume();
            ASTNode* Right = ParseRelational();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<ComparisonNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseRelational()
    {
        ASTNode* Left = ParseShift();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetRelationalOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
                break;
            Consume();
            ASTNode* Right = ParseShift();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<ComparisonNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseShift()
    {
        ASTNode* Left = ParseAdditive();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetShiftOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
                break;
            Consume();
            ASTNode* Right = ParseAdditive();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseAdditive()
    {
        ASTNode* Left = ParseMultiplicative();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetAdditiveOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
                break;
            Consume();
            ASTNode* Right = ParseMultiplicative();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseMultiplicative()
    {
        ASTNode* Left = ParseUnary();
        if (!IsValidNode(Left))
            return Left;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            OperatorType OpType = Operator::GetMultiplicativeOp(Tok.Type);
            if (OpType == OperatorType::UNKNOWN)
                break;
            Consume();
            ASTNode* Right = ParseUnary();
            if (!IsValidNode(Right))
                return Right;

            Left = NodesArena.Create<BinaryOpNode>(
                OpType, Left, Right, Left->Pos, Left->Line, Left->Column);
        }

        return Left;
    }

    ASTNode* Parser::ParseUnary()
    {
        if (!IsValidIndex())
            return nullptr;

        const Token& Tok = CurrentToken();
        OperatorType OpType = Operator::GetUnaryOp(Tok.Type);
        if (OpType != OperatorType::UNKNOWN)
        {
            Consume();
            ASTNode* Operand = ParseUnary();
            if (!IsValidNode(Operand))
                return Operand;

            if (OpType == OperatorType::INC || OpType == OperatorType::DEC)
                return NodesArena.Create<PrefixOpNode>(
                    OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);

            if (OpType == OperatorType::MUL)
                return NodesArena.Create<UnrefNode>(
                    Operand, Operand->Pos, Operand->Line, Operand->Column);

            return NodesArena.Create<UnaryOpNode>(
                OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);
        }

        if (Tok.Type == TokenType::OP_REFERENCE)
        {
            Consume();
            ASTNode* Target = ParseUnary();
            if (!IsValidNode(Target))
                return Target;

            return NodesArena.Create<RefNode>(Target, Tok.Pos, Tok.Line, Tok.Column);
        }

        return ParsePostfix();
    }

    ASTNode* Parser::ParsePostfix()
    {
        ASTNode* Operand = ParsePrimary();
        if (!IsValidNode(Operand))
            return Operand;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            TokenType TokType = Tok.Type;
            switch (TokType)
            {
                case TokenType::OP_LPAREN:
                {
                    if (auto Type = Cast<ClassTypeNode>(Operand))
                    {
                        Consume();

                        ArgsVector<ASTNode*> Arguments;
                        if (!ParseExpressionsSeparatedByComma(Arguments, TokenType::OP_RPAREN))
                        {
                            SkipExpressionInBrackets(TokenType::OP_LPAREN,
                                TokenType::OP_RPAREN, 1);
                            return CreateErrorNode(Type->Pos, Type->Line, Type->Column);
                        }

                        if (!Expect(TokenType::OP_RPAREN))
                            return CreateErrorNode(Type->Pos, Type->Line, Type->Column);

                        Operand = NodesArena.Create<ConstructNode>(Type, std::move(Arguments),
                            Type->Pos, Type->Line, Type->Column);
                        break;
                    }

                    if (auto Type = Cast<DataTypeNodeBase>(Operand))
                    {
                        Consume();
                        ASTNode* Value = ParseAssignment();
                        if (!Value) return nullptr;
                        if (IsErrorNode(Value))
                        {
                            SkipExpressionInBrackets(TokenType::OP_LPAREN,
                                TokenType::OP_RPAREN, 1);
                            return Value;
                        }

                        Operand = NodesArena.Create<ExplicitCastNode>(
                            Type, Value, Type->Pos, Type->Line, Type->Column);

                        if (!Expect(TokenType::OP_RPAREN))
                            return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);

                        break;
                    }

                    Consume();
                    ArgsVector<ASTNode*> Arguments;
                    if (!ParseExpressionsSeparatedByComma(Arguments, TokenType::OP_RPAREN))
                    {
                        SkipExpressionInBrackets(TokenType::OP_LPAREN,
                            TokenType::OP_RPAREN, 1);
                        return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);
                    }

                    if (!Expect(TokenType::OP_RPAREN))
                        return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);

                    Operand = NodesArena.Create<CallNode>(Operand, std::move(Arguments),
                        Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                case TokenType::OP_LBRACKET:
                {
                    Consume();
                    ASTNode* Index = ParseAssignment();
                    if (!IsValidNode(Index)) return Index;

                    if (!Expect(TokenType::OP_RBRACKET))
                        return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);

                    Operand = NodesArena.Create<SubscriptNode>(
                        Operand, Index, Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                case TokenType::OP_DOT:
                {
                    Consume();
                    Operand = NodesArena.Create<MemberAccessNode>(
                        Operand, ParsePrimary(), Operand->Pos, Operand->Line, Operand->Column);
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
        using enum TokenType;
        if (!IsValidIndex())
            return nullptr;

        const Token& Tok = CurrentToken();

        switch (Tok.Type)
        {
            case IDENTIFIER:
            {
                llvm::StringRef Lexeme = GetTokenLexeme(Tok);
                if (CustomTypes.contains(Lexeme))
                    return ParseDataType();

                Consume();
                return NodesArena.Create<IdentifierNode>(
                    Lexeme, Tok.Pos, Tok.Line, Tok.Column);
            }

            case I8_NUMBER:  case I16_NUMBER:
            case I32_NUMBER: case I64_NUMBER:
            case U8_NUMBER:  case U16_NUMBER:
            case U32_NUMBER: case U64_NUMBER:
                Consume();
                return CreateInteger(Tok);
            case F16_NUMBER: case F32_NUMBER:
            case F64_NUMBER: case F128_NUMBER:
                Consume();
                return CreateFloatingPoint(Tok);
            case BOOL_TRUE:
                Consume();
                return NodesArena.Create<BoolNode>(true, Tok.Pos, Tok.Line, Tok.Column);
            case BOOL_FALSE:
                Consume();
                return NodesArena.Create<BoolNode>(false, Tok.Pos, Tok.Line, Tok.Column);
            case CHAR:
                Consume();
                return NodesArena.Create<CharNode>(
                    GetTokenLexeme(Tok)[0], Tok.Pos, Tok.Line, Tok.Column);
            case STRING:
                Consume();
                return NodesArena.Create<StringNode>(
                    GetTokenLexeme(Tok), Tok.Pos, Tok.Line, Tok.Column);
            case NULL_POINTER:
                Consume();
                return NodesArena.Create<NullPointerNode>(Tok.Pos, Tok.Line, Tok.Column);
            case OP_LPAREN:
            {
                Consume();
                ASTNode* Node = ParseAssignment();
                if (!Node) return nullptr;
                if (IsErrorNode(Node))
                {
                    SkipExpressionInBrackets(OP_LPAREN, OP_RPAREN, 1);
                    return Node;
                }

                if (!Expect(OP_RPAREN))
                    return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
                return Node;
            }
            case OP_LBRACKET:
            {
                Consume();
                SmallVec16<ASTNode*> Elements;
                while (IsValidIndex())
                {
                    if (CurrentToken().Type == OP_RBRACKET)
                        break;
                    ASTNode* El = ParseAssignment();
                    if (!IsValidNode(El))
                    {
                        SkipExpressionInBrackets(OP_LBRACKET,
                            OP_RBRACKET, 1);
                        return El;
                    }

                    Elements.push_back(El);
                    if (!ConsumeIf(OP_COMMA))
                        break;
                }
                if (!Expect(OP_RBRACKET))
                    return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
                return NodesArena.Create<ArrayNode>(std::move(Elements), Tok.Pos, Tok.Line, Tok.Column);
            }
            case OP_LBRACE:
                return ParseBlock();
            default:
                break;
        }

        if (auto Type = ParseDataType())
            return Type;

        SendError(ParseErrorType::UnexpectedToken, Tok.Line, Tok.Column,
            { GetTokenLexeme(Tok).str() });
        Consume();

        return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
    }

    ASTNode* Parser::CreateInteger(const Token &Tok) const
    {
        using enum TokenType;

        size_t BitWidth;
        bool IsSigned = true;
        switch (Tok.Type)
        {
            case I8_NUMBER:  BitWidth = 8;  break;
            case I16_NUMBER: BitWidth = 16; break;
            case I32_NUMBER: BitWidth = 32; break;
            case I64_NUMBER: BitWidth = 64; break;
            case U8_NUMBER:  BitWidth = 8;  IsSigned = false; break;
            case U16_NUMBER: BitWidth = 16; IsSigned = false; break;
            case U32_NUMBER: BitWidth = 32; IsSigned = false; break;
            case U64_NUMBER: BitWidth = 64; IsSigned = false; break;
            default: VoltUnreachable("Invalid integer type");
        }

        UInt64 Value;
        llvm::StringRef NumStr = GetTokenLexeme(Tok);
        std::from_chars(NumStr.data(), NumStr.data() + NumStr.size(), Value);
        return NodesArena.Create<IntegerNode>(
            BitWidth, Value, IsSigned, Tok.Pos, Tok.Line, Tok.Column);
    }

    ASTNode* Parser::CreateFloatingPoint(const Token &Tok) const
    {
        using enum TokenType;

        size_t BitWidth;
        switch (Tok.Type)
        {
            case F16_NUMBER:  BitWidth = 16;  break;
            case F32_NUMBER:  BitWidth = 32;  break;
            case F64_NUMBER:  BitWidth = 64;  break;
            case F128_NUMBER: BitWidth = 128; break;
            default: VoltUnreachable("Invalid floating point type");
        }

        double Value;
        llvm::StringRef NumStr = GetTokenLexeme(Tok);
        std::from_chars(NumStr.data(), NumStr.data() + NumStr.size(), Value);
        return NodesArena.Create<FloatingPointNode>(
            BitWidth, Value, Tok.Pos, Tok.Line, Tok.Column);
    }

    bool Parser::ParseExpressionsSeparatedByComma(ArgsVector<ASTNode*> &Nodes, TokenType StopToken)
    {
        while (IsValidIndex())
        {
            if (CurrentToken().Type == StopToken)
                return true;

            if (ASTNode* Arg = ParseAssignment())
            {
                if (IsErrorNode(Arg))
                    return false;
                Nodes.push_back(Arg);
            }
            else
                return false;

            if (!ConsumeIf(TokenType::OP_COMMA))
                break;
        }

        return true;
    }

    void Parser::ParseParametersInParens(ArgsVector<ParamNode *> &Params)
    {
        Expect(TokenType::OP_LPAREN);

        while (IsValidIndex())
        {
            if (CurrentToken().Type == TokenType::OP_RPAREN)
                break;
            if (auto Node = ParseParameter())
            {
                if (IsErrorNode(Node))
                {
                    TokenType TokenTy = JumpToOneOfTokens(TokenType::OP_COLON, TokenType::OP_RPAREN,
                        TokenType::OP_LBRACE, TokenType::KW_LET, TokenType::KW_FUN, TokenType::KW_CLASS);
                    if (TokenTy == TokenType::OP_COLON || TokenTy == TokenType::OP_RPAREN)
                        continue;
                    break;
                }

                auto Parameter = Cast<ParamNode>(Node);

                if (std::find_if(Params.begin(), Params.end(),
                    [Parameter](const ParamNode* Value) {
                        return Parameter->Name == Value->Name;
                    }) != Params.end())
                {
                    // SendError
                }

                if (!Parameter->DefaultValue)
                {
                    if (std::find_if(Params.begin(), Params.end(),
                        [](const ParamNode* Value) -> bool {
                            return Value->DefaultValue;
                        }) != Params.end())
                    {
                        // SendError
                    }
                }

                Params.push_back(Parameter);
            }
            else if (!ConsumeIf(TokenType::OP_COMMA))
                break;
        }

        Expect(TokenType::OP_RPAREN);
    }

    bool Parser::SkipToToken(TokenType Type)
    {
        while (IsValidIndex())
        {
            if (CurrentToken().Type == Type)
                return true;
            Consume();
        }

        return false;
    }
}
