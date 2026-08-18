//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_PARSER_H
#define CVOLT_PARSER_H

#include "Volt/Core/Lexer/Lexer.h"
#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/Core/Errors/ParseError.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include <llvm/ADT/StringSet.h>

namespace Volt
{
    class Parser
    {
    private:
        CompilationContext& CContext;
        Arena& NodesArena;

        const Array<Token>& Tokens;

        size_t Index = 0;
        ASTNode*& Root;

        Array<ParseError>& Errors;

        bool LastNodeIsBlock = false;
        bool InBlock = false;
        bool InFunction = false;
        bool InLoop = false;

        llvm::StringSet<> CustomTypes;
        size_t Depth = 0;
    public:
        Parser(CompilationContext& CContext)
            : CContext(CContext), NodesArena(CContext.MainArena),
            Tokens(CContext.Tokens), Root(CContext.ASTTree), Errors(CContext.ParseErrors) {}

        void Parse();

    private:
        [[nodiscard]] bool IsValidIndex() const { return Index < Tokens.Length(); }
        [[nodiscard]] const Token& CurrentToken() const
        {
            VoltAssert(Index < Tokens.Length() && "Out of array range");
            return Tokens[Index];
        }
        [[nodiscard]] const Token& PrevToken() const
        {
            VoltAssert(Index - 1 < Tokens.Length() && "Out of array range");
            return Tokens[Index - 1];
        }
        [[nodiscard]] const Token& CurrentOrEndToken() const
        {
            VoltAssert(!Tokens.Empty() && "Tokens list is empty");
            return Index < Tokens.Length() ? Tokens[Index] : Tokens.Back();
        }

        bool Consume();
        void SkipSemicolons();
        void SkipExpressionInBrackets(TokenType OpenBracket, TokenType CloseBracket, int ConsumedBrackets = 0);
        void Synchronize();
        void JumpToNextGlobalDeclaration();
        bool GetTokenIf(size_t Index, TokenType Type, const Token*& TokPtr) const;
        bool GetNextTokenIf(TokenType Type, const Token*& TokPtr, size_t NextIndexOffset = 1) const;
        bool Peek(TokenType Type, const Token*& TokPtr) const;
        [[nodiscard]] bool Peek(TokenType Type) const;
        bool ConsumeIf(TokenType Type, const Token*& TokPtr);
        bool ConsumeIf(TokenType Type);
        bool Expect(TokenType Type);

        bool ExpectAndConsume(TokenType Type);

        [[nodiscard]] llvm::StringRef GetTokenLexeme(const Token& Tok) const
        {
            return CContext.GetTokenLexeme(Tok.Lexeme);
        }

        void SendError(ParseErrorType Type, size_t Line, size_t Column, Array<std::string>&& Context = {});
        void SendError(ParseErrorType Type, Array<std::string>&& Context = {});

    private:
        ASTNode* ParseSequence();
        ASTNode* ParseBlock();

        ASTNode* ParseDataType() { return ParseReferenceType(); }
        ASTNode* ParseReferenceType();
        ASTNode* ParseQualType();
        ASTNode* ParseWrappedType();
        ASTNode* ParsePrimitiveType();

        ASTNode* ParseParameter();
        ASTNode* ParseFunction();
        ASTNode* ParseVariable();
        ASTNode* ParseClass();
        ASTNode* ParseField();
        ASTNode* ParseIf();
        ASTNode* ParseWhile();
        ASTNode* ParseFor();
        ASTNode* ParseForInitialization();
        ASTNode* ParseForCondition();
        ASTNode* ParseFotIteration();
        ASTNode* ParseReturn();
        ASTNode* ParseBreak();
        ASTNode* ParseContinue();

        ASTNode* ParseExpression();
        ASTNode* ParseStatement();
        ASTNode* ParseAssignment();
        ASTNode* ParseLogicalOR();
        ASTNode* ParseLogicalAND();
        ASTNode* ParseBitwiseOR();
        ASTNode* ParseBitwiseXOR();
        ASTNode* ParseBitwiseAND();
        ASTNode* ParseEquality();
        ASTNode* ParseRelational();
        ASTNode* ParseShift();
        ASTNode* ParseAdditive();
        ASTNode* ParseMultiplicative();
        ASTNode* ParseUnary();
        ASTNode* ParsePostfix();
        ASTNode* ParsePrimary();

        [[nodiscard]] ASTNode* CreateInteger(const Token& Tok) const;
        [[nodiscard]] ASTNode* CreateFloatingPoint(const Token& Tok) const;

        bool ParseExpressionsSeparatedByComma(ArgsVector<ASTNode*>& Nodes, TokenType StopToken);
        void ParseParametersInParens(ArgsVector<ParamNode*>& Params);
        bool SkipToToken(TokenType Type);

        template <typename ...Args_>
        TokenType JumpToOneOfTokens(Args_... Types);

        [[nodiscard]] ErrorNode* CreateErrorNode(size_t Pos, size_t Line, size_t Column) const
        {
            return NodesArena.Create<ErrorNode>(Pos, Line, Column);
        }
        [[nodiscard]] ErrorNode* CreateErrorNodeOnCurrentOrEndToken() const
        {
            const Token& Tok = CurrentOrEndToken();
            return NodesArena.Create<ErrorNode>(Tok.Pos, Tok.Line, Tok.Column);
        }

        [[nodiscard]] static bool IsErrorNode(ASTNode* Node) { return IsA<ErrorNode>(Node); }
        [[nodiscard]] static bool IsValidNode(ASTNode* Node) { return Node && !IsErrorNode(Node); }
    };

    template<typename ... Args_>
    TokenType Parser::JumpToOneOfTokens(Args_... Types)
    {
        using enum TokenType;

        while (IsValidIndex())
        {
            const Token& Tok = CurrentToken();
            if (Tok.IsOneOf(Types...))
                return Tok.Type;

            switch (Tok.Type)
            {
                case LParen:
                    SkipExpressionInBrackets(LParen, RParen);
                    break;
                case LSquare:
                    SkipExpressionInBrackets(LSquare, RSquare);
                    break;
                case LBrace:
                    SkipExpressionInBrackets(LBrace, RBrace);
                    break;
                default:
                    Consume();
            }
        }

        return Unknown;
    }
}

#endif //CVOLT_PARSER_H