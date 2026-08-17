//
// Created by bohdan on 14.12.25.
//

#include "Volt/Core/Parser/Parser.h"
#include "Volt/Core/TypeChecker/ExprResult.h"

#include <charconv>

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
            if (CurrentToken().Type != TokenType::Semicolon)
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
                case TokenType::Semicolon:
                case TokenType::KwLet:
                case TokenType::KwIf:
                case TokenType::KwWhile:
                case TokenType::KwFor:
                case TokenType::KwReturn:
                case TokenType::KwBreak:
                case TokenType::KwContinue:
                case TokenType::LBrace:
                case TokenType::RBrace:
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
            if (Tok.Type == TokenType::LBrace)
            {
                Consume();
                BlocksCount++;
                while (IsValidIndex())
                {
                    if (BlocksCount == 0)
                        break;

                    if (Peek(TokenType::LBrace))
                        BlocksCount++;
                    if (Peek(TokenType::RBrace))
                        BlocksCount--;

                    Consume();
                }

                if (BlocksCount != 0)
                    SendError(ParseErrorType::UnexpectedEOF, PrevToken().Line, PrevToken().Column);

                break;
            }

            switch (Tok.Type)
            {
                case TokenType::KwFun:
                case TokenType::KwLet:
                case TokenType::KwClass:
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
        if (!ConsumeIf(TokenType::LBrace, TokPtr))
        {
            while (IsValidIndex() && CurrentToken().Type != TokenType::RBrace)
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
            if (ConsumeIf(TokenType::RBrace))
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
        Expect(TokenType::RBrace);
        return nullptr;
    }

    ASTNode *Parser::ParseReferenceType()
    {
        ASTNode* TypeNode = ParseQualType();
        if (!IsValidNode(TypeNode))
            return TypeNode;

        if (ConsumeIf(TokenType::Dollar))
            return NodesArena.Create<ReferenceTypeNode>(
                Cast<DataTypeNodeBase>(TypeNode), TypeNode->Pos, TypeNode->Line, TypeNode->Column);

        return TypeNode;
    }

    ASTNode *Parser::ParseQualType()
    {
        if (!IsValidIndex())
            return nullptr;

        const Token* TokPtr;
        if (ConsumeIf(TokenType::TypeConst, TokPtr))
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
                case TokenType::Star:
                    TypeNode = NodesArena.Create<PointerTypeNode>(
                        Cast<DataTypeNodeBase>(TypeNode), Tok.Pos, Tok.Line, Tok.Column);
                    Consume();
                    break;
                case TokenType::LSquare:
                {
                    Consume();
                    ASTNode* Length = ParseAssignment();
                    if (!Expect(TokenType::RSquare))
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
            case TypeVoid: Type = CContext.GetVoidType(); break;
            case TypeBool: Type = CContext.GetBoolType(); break;
            case TypeChar: Type = CContext.GetCharType(); break;
            case TypeI8:   Type = CContext.GetIntegerType(8, true);   break;
            case TypeI16:  Type = CContext.GetIntegerType(16, true);  break;
            case TypeI32:  Type = CContext.GetIntegerType(32, true);  break;
            case TypeI64:  Type = CContext.GetIntegerType(64, true);  break;
            case TypeU8:   Type = CContext.GetIntegerType(8, false);  break;
            case TypeU16:  Type = CContext.GetIntegerType(16, false); break;
            case TypeU32:  Type = CContext.GetIntegerType(32, false); break;
            case TypeU64:  Type = CContext.GetIntegerType(64, false); break;
            case TypeF16:  Type = CContext.GetFPType(16);  break;
            case TypeF32:  Type = CContext.GetFPType(32);  break;
            case TypeF64:  Type = CContext.GetFPType(64);  break;
            case TypeF128: Type = CContext.GetFPType(128); break;
            case Identifier:
            {
                // llvm::StringRef Lexeme = GetTokenLexeme(Tok);
                // if (!CustomTypes.contains(Lexeme) && !CContext.GetClassType(Lexeme)) return nullptr;
                Consume();
                return NodesArena.Create<ClassTypeNode>(GetTokenLexeme(Tok), Tok.Pos, Tok.Line, Tok.Column);
            }
            case LParen:
            {
                Consume();
                ASTNode* Node = ParseQualType();
                Expect(RParen);
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

        if (!ConsumeIf(TokenType::Identifier, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            return CreateErrorNodeOnCurrentOrEndToken();
        }

        llvm::StringRef Name = GetTokenLexeme(*TokPtr);

        DataTypeNodeBase* DataTypeNode = Cast<DataTypeNodeBase>(DataType);
        if (!ConsumeIf(TokenType::Equal))
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
        if (!ConsumeIf(TokenType::KwFun, FirstTokPtr))
            return nullptr;

        Expect(TokenType::Colon);

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
        if (!ConsumeIf(TokenType::Identifier, TokPtr))
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
        if (!ConsumeIf(TokenType::KwLet, FirstTokPtr))
            return nullptr;

        Expect(TokenType::Colon);

        ASTNode* DataType = ParseDataType();
        if (IsErrorNode(DataType))
        {
            JumpToOneOfTokens(TokenType::Identifier);
        }

        if (!DataType)
        {
            SendError(ParseErrorType::ExpectedDataType);
            Synchronize();
            return CreateErrorNode(FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
        }

        const Token* TokPtr;
        if (!ConsumeIf(TokenType::Identifier, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return CreateErrorNode(FirstTokPtr->Pos, FirstTokPtr->Line, FirstTokPtr->Column);
        }
        llvm::StringRef Name = GetTokenLexeme(*TokPtr);

        DataTypeNodeBase* DataTypeNode = Cast<DataTypeNodeBase>(DataType);
        if (ConsumeIf(TokenType::Equal))
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

        if (ConsumeIf(TokenType::LParen))
        {
            ArgsVector<ASTNode*> Args;
            ParseExpressionsSeparatedByComma(Args, TokenType::RParen);
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
        if (!ConsumeIf(TokenType::KwClass, FirstTokPtr))
            return nullptr;

        const Token* TokPtr;
        if (!ConsumeIf(TokenType::Identifier, TokPtr))
        {
            SendError(ParseErrorType::ExpectedDeclaratorName);
            Synchronize();
            return nullptr;
        }
        llvm::StringRef Name = GetTokenLexeme(*TokPtr);
        CustomTypes.insert(Name);

        Expect(TokenType::LBrace);

        SmallVec4<VariableNode*> Fields;
        SmallVec4<FunctionNode*> Methods;
        SmallVec4<ConstructorNode*> Constructors;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            TokenType TokType = Tok.Type;
            if (TokType == TokenType::RBrace)
                break;

            switch (TokType)
            {
                case TokenType::KwLet:
                {
                    if (VariableNode* Field = Cast<VariableNode>(ParseVariable()))
                    {
                        Fields.push_back(Field);
                        Expect(TokenType::Semicolon);
                        SkipSemicolons();
                        break;
                    }
                    return nullptr;
                }
                case TokenType::KwFun:
                {
                    if (FunctionNode* Method = Cast<FunctionNode>(ParseFunction()))
                    {
                        Methods.push_back(Method);
                        break;
                    }
                    return nullptr;
                }
                case TokenType::Identifier:
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

        Expect(TokenType::RBrace);

        return NodesArena.Create<ClassNode>(Name, std::move(Fields), std::move(Methods),
                                            std::move(Constructors), FirstTokPtr->Pos,
                                            FirstTokPtr->Line, FirstTokPtr->Column);
    }

    ASTNode* Parser::ParseIf()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KwIf, TokPtr))
            return nullptr;

        Expect(TokenType::LParen);

        ASTNode* Condition = nullptr;
        if (ConsumeIf(TokenType::RParen))
            SendError(ParseErrorType::ExpectedExpression);
        else
            Condition = ParseAssignment();

        Expect(TokenType::RParen);

        ASTNode* Branch = nullptr;
        if (CurrentToken().Type == TokenType::LBrace)
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
        if (!ConsumeIf(TokenType::KwElse))
            return If;

        ASTNode* ElseBranch = nullptr;
        if (CurrentToken().Type == TokenType::LBrace)
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
        if (!ConsumeIf(TokenType::KwWhile, TokPtr))
            return nullptr;

        Expect(TokenType::LParen);

        ASTNode* Condition = nullptr;
        if (ConsumeIf(TokenType::RParen))
            SendError(ParseErrorType::ExpectedExpression);
        else
            Condition = ParseAssignment();

        Expect(TokenType::RParen);

        ASTNode* Branch = nullptr;
        bool OldInLoop = InLoop;
        InLoop = true;
        if (CurrentToken().Type == TokenType::LBrace)
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
        if (!ConsumeIf(TokenType::KwFor, TokPtr))
            return nullptr;

        Expect(TokenType::LParen);

        ASTNode* Initialization = ParseForInitialization();
        ASTNode* Condition =      ParseForCondition();
        ASTNode* Iteration =      ParseFotIteration();

        ASTNode* Body = nullptr;
        bool OldInLoop = InLoop;
        InLoop = true;
        if (Peek(TokenType::LBrace))
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
        if (ConsumeIf(TokenType::Semicolon))
            return nullptr;

        ASTNode* Initialization = ParseStatement();
        if (!Initialization)
            SendError(ParseErrorType::ExpectedInitializerExpression);

        Expect(TokenType::Semicolon);
        return Initialization;
    }

    ASTNode *Parser::ParseForCondition()
    {
        if (ConsumeIf(TokenType::Semicolon))
            return nullptr;

        ASTNode* Condition = ParseAssignment();
        if (!Condition)
            SendError(ParseErrorType::ExpectedExpression);

        Expect(TokenType::Semicolon);
        return Condition;
    }

    ASTNode *Parser::ParseFotIteration()
    {
        if (ConsumeIf(TokenType::RParen))
            return nullptr;

        ASTNode* Iteration = ParseAssignment();
        if (!Iteration)
            SendError(ParseErrorType::ExpectedExpression);

        Expect(TokenType::RParen);
        return Iteration;
    }

    ASTNode* Parser::ParseReturn()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KwReturn, TokPtr))
            return nullptr;

        if (!InFunction)
        {
            SendError(ParseErrorType::ReturnOutsideFunction, TokPtr->Line, TokPtr->Column);
            Synchronize();
            return CreateErrorNode(TokPtr->Pos, TokPtr->Line, TokPtr->Column);
        }

        if (Peek(TokenType::Semicolon))
            return NodesArena.Create<ReturnNode>(
                nullptr, TokPtr->Pos, TokPtr->Line, TokPtr->Column);

        return NodesArena.Create<ReturnNode>(
            ParseAssignment(), TokPtr->Pos, TokPtr->Line, TokPtr->Column);
    }

    ASTNode* Parser::ParseBreak()
    {
        const Token* TokPtr = nullptr;
        if (!ConsumeIf(TokenType::KwBreak ,TokPtr))
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
        if (!ConsumeIf(TokenType::KwContinue, TokPtr))
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

        Expect(TokenType::Semicolon);
        SkipSemicolons();
        return Node;
    }

    ASTNode* Parser::ParseStatement()
    {
        if (!IsValidIndex())
            return nullptr;

        const Token& Tok = CurrentToken();
        if (Tok.Type == TokenType::KwFun)
        {
            if (InBlock)
            {
                SendError(ParseErrorType::FunctionDefinitionNotAllowed);
                Synchronize();
                return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
            }

            return ParseFunction();
        }

        if (Tok.Type == TokenType::KwClass)
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
                case TokenType::KwIf:
                    return ParseIf();
                case TokenType::KwWhile:
                    return ParseWhile();
                case TokenType::KwFor:
                    return ParseFor();
                case TokenType::KwReturn:
                    return ParseReturn();
                case TokenType::KwBreak:
                    return ParseBreak();
                case TokenType::KwContinue:
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
            if (OpType == OperatorType::Unknown)
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
            if (OpType != OperatorType::LogicalOr)
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
            if (OpType != OperatorType::LogicalAnd)
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
            if (OpType != OperatorType::BitOr)
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
            if (OpType != OperatorType::BitXor)
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
            if (OpType != OperatorType::BitAnd)
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
            if (OpType == OperatorType::Unknown)
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
            if (OpType == OperatorType::Unknown)
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
            if (OpType == OperatorType::Unknown)
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
            if (OpType == OperatorType::Unknown)
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
            if (OpType == OperatorType::Unknown)
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
        if (OpType != OperatorType::Unknown)
        {
            Consume();
            ASTNode* Operand = ParseUnary();
            if (!IsValidNode(Operand))
                return Operand;

            if (OpType == OperatorType::Inc || OpType == OperatorType::Dec)
                return NodesArena.Create<PrefixOpNode>(
                    OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);

            if (OpType == OperatorType::Unref)
                return NodesArena.Create<UnrefNode>(
                    Operand, Operand->Pos, Operand->Line, Operand->Column);

            return NodesArena.Create<UnaryOpNode>(
                OpType, Operand, Tok.Pos, Tok.Line, Tok.Column);
        }

        if (Tok.Type == TokenType::Dollar)
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
                case TokenType::LParen:
                {
                    if (auto Type = Cast<ClassTypeNode>(Operand))
                    {
                        Consume();

                        ArgsVector<ASTNode*> Arguments;
                        if (!ParseExpressionsSeparatedByComma(Arguments, TokenType::RParen))
                        {
                            SkipExpressionInBrackets(TokenType::LParen,
                                TokenType::RParen, 1);
                            return CreateErrorNode(Type->Pos, Type->Line, Type->Column);
                        }

                        if (!Expect(TokenType::RParen))
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
                            SkipExpressionInBrackets(TokenType::LParen,
                                TokenType::RParen, 1);
                            return Value;
                        }

                        Operand = NodesArena.Create<ExplicitCastNode>(
                            Type, Value, Type->Pos, Type->Line, Type->Column);

                        if (!Expect(TokenType::RParen))
                            return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);

                        break;
                    }

                    Consume();
                    ArgsVector<ASTNode*> Arguments;
                    if (!ParseExpressionsSeparatedByComma(Arguments, TokenType::RParen))
                    {
                        SkipExpressionInBrackets(TokenType::LParen,
                            TokenType::RParen, 1);
                        return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);
                    }

                    if (!Expect(TokenType::RParen))
                        return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);

                    Operand = NodesArena.Create<CallNode>(Operand, std::move(Arguments),
                        Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                case TokenType::LSquare:
                {
                    Consume();
                    ASTNode* Index = ParseAssignment();
                    if (!IsValidNode(Index)) return Index;

                    if (!Expect(TokenType::RSquare))
                        return CreateErrorNode(Operand->Pos, Operand->Line, Operand->Column);

                    Operand = NodesArena.Create<SubscriptNode>(
                        Operand, Index, Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                case TokenType::Dot:
                {
                    Consume();
                    Operand = NodesArena.Create<MemberAccessNode>(
                        Operand, ParsePrimary(), Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                case TokenType::KwTo:
                {
                    Consume();
                    ASTNode* Node = ParseDataType();
                    if (!IsValidNode(Node)) return Node;
                    Operand = NodesArena.Create<ExplicitCastNode>(StaticCast<DataTypeNodeBase>(Node),
                        Operand, Operand->Pos, Operand->Line, Operand->Column);
                    break;
                }
                default:
                {
                    OperatorType OpType = Operator::GetPostfix(TokType);
                    if (OpType == OperatorType::Unknown)
                        return Operand;
                    Consume();

                    if (OpType == OperatorType::Inc || OpType == OperatorType::Dec)
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
            case Identifier:
            {
                llvm::StringRef Lexeme = GetTokenLexeme(Tok);
                if (CustomTypes.contains(Lexeme))
                    return ParseDataType();

                Consume();
                return NodesArena.Create<IdentifierNode>(
                    Lexeme, Tok.Pos, Tok.Line, Tok.Column);
            }

            case NumberI8:  case NumberI16:
            case NumberI32: case NumberI64:
            case NumberU8:  case NumberU16:
            case NumberU32: case NumberU64:
                Consume();
                return CreateInteger(Tok);
            case NumberF16: case NumberF32:
            case NumberF64: case NumberF128:
                Consume();
                return CreateFloatingPoint(Tok);
            case BoolTrue:
                Consume();
                return NodesArena.Create<BoolNode>(true, Tok.Pos, Tok.Line, Tok.Column);
            case BoolFalse:
                Consume();
                return NodesArena.Create<BoolNode>(false, Tok.Pos, Tok.Line, Tok.Column);
            case Char:
                Consume();
                return NodesArena.Create<CharNode>(
                    GetTokenLexeme(Tok)[0], Tok.Pos, Tok.Line, Tok.Column);
            case String:
                Consume();
                return NodesArena.Create<StringNode>(
                    GetTokenLexeme(Tok), Tok.Pos, Tok.Line, Tok.Column);
            case NullPointer:
                Consume();
                return NodesArena.Create<NullPointerNode>(Tok.Pos, Tok.Line, Tok.Column);
            case LParen:
            {
                Consume();
                ASTNode* Node = ParseAssignment();
                if (!Node) return nullptr;
                if (IsErrorNode(Node))
                {
                    SkipExpressionInBrackets(LParen, RParen, 1);
                    return Node;
                }

                if (!Expect(RParen))
                    return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
                return Node;
            }
            case LSquare:
            {
                Consume();
                SmallVec16<ASTNode*> Elements;
                while (IsValidIndex())
                {
                    if (CurrentToken().Type == RSquare)
                        break;
                    ASTNode* El = ParseAssignment();
                    if (!IsValidNode(El))
                    {
                        SkipExpressionInBrackets(LSquare,
                            RSquare, 1);
                        return El;
                    }

                    Elements.push_back(El);
                    if (!ConsumeIf(Comma))
                        break;
                }
                if (!Expect(RSquare))
                    return CreateErrorNode(Tok.Pos, Tok.Line, Tok.Column);
                return NodesArena.Create<ArrayNode>(std::move(Elements), Tok.Pos, Tok.Line, Tok.Column);
            }
            case LBrace:
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
            case NumberI8:  BitWidth = 8;  break;
            case NumberI16: BitWidth = 16; break;
            case NumberI32: BitWidth = 32; break;
            case NumberI64: BitWidth = 64; break;
            case NumberU8:  BitWidth = 8;  IsSigned = false; break;
            case NumberU16: BitWidth = 16; IsSigned = false; break;
            case NumberU32: BitWidth = 32; IsSigned = false; break;
            case NumberU64: BitWidth = 64; IsSigned = false; break;
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
            case NumberF16:  BitWidth = 16;  break;
            case NumberF32:  BitWidth = 32;  break;
            case NumberF64:  BitWidth = 64;  break;
            case NumberF128: BitWidth = 128; break;
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

            if (!ConsumeIf(TokenType::Comma))
                break;
        }

        return true;
    }

    void Parser::ParseParametersInParens(ArgsVector<ParamNode *> &Params)
    {
        Expect(TokenType::LParen);

        while (IsValidIndex())
        {
            if (CurrentToken().Type == TokenType::RParen)
                break;
            if (auto Node = ParseParameter())
            {
                if (IsErrorNode(Node))
                {
                    TokenType TokenTy = JumpToOneOfTokens(TokenType::Colon, TokenType::RParen,
                        TokenType::LBrace, TokenType::KwLet, TokenType::KwFun, TokenType::KwClass);
                    if (TokenTy == TokenType::Colon || TokenTy == TokenType::RParen)
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
            else if (!ConsumeIf(TokenType::Comma))
                break;
        }

        Expect(TokenType::RParen);
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
