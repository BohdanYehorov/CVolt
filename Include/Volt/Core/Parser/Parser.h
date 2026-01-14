//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_PARSER_H
#define CVOLT_PARSER_H

#include "Volt/Core/Lexer/Lexer.h"
#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/Core/Errors/Errors.h"
#include <ostream>

namespace Volt
{
    class Parser
    {
    private:
        struct DepthIncScope
        {
            size_t& Depth;
            DepthIncScope(size_t& Depth) : Depth(Depth)
            {
                if (++Depth > 10000)
                    throw std::runtime_error("Big depth");
            }

            ~DepthIncScope()
            {
                --Depth;
            }
        };

    private:
        Arena NodesArena;
        const ArenaStream& TokensArena;

        const std::vector<Token>& Tokens;
        std::vector<ParseError> Errors;

        size_t Index = 0;
        ASTNode* Root = nullptr;

        bool LastNodeIsBlock = false;
        bool InBlock = false;
        bool InFunction = false;
        bool InLoop = false;

        size_t Depth = 0;

    private:
        static void PrintASTTree(std::ostream& Os, ASTNode* Node, int Tabs = 0);

    public:
        Parser(const Lexer& L) : TokensArena(L.GetTokensArena()), Tokens(L.GetTokens()) {}

        void Parse();
        [[nodiscard]] ASTNode* GetASTTree() const { return Root; }
        [[nodiscard]] const std::vector<ParseError>& GetErrorList() const { return Errors; }
        [[nodiscard]] bool HasErrors() const { return !Errors.empty(); }
        bool PrintErrors() const;
        void PrintASTTree(std::ostream& Os) const;

    private:
        [[nodiscard]] bool IsValidIndex() const { return Index < Tokens.size(); }
        [[nodiscard]] const Token& CurrentToken() const { return Tokens[Index]; }
        [[nodiscard]] const Token& PrevToken() const { return Tokens[Index - 1]; }
        bool Consume();
        void SkipSemicolons();
        void Synchronize();
        void JumpToNextGlobalDeclaration();
        bool GetTokenIf(size_t Index, Token::TokenType Type, const Token*& TokPtr) const;
        bool GetNextTokenIf(Token::TokenType Type, const Token*& TokPtr, size_t NextIndexOffset = 1) const;
        bool Peek(Token::TokenType Type, const Token*& TokPtr) const;
        [[nodiscard]] bool Peek(Token::TokenType Type) const;
        bool ConsumeIf(Token::TokenType Type, const Token*& TokPtr);
        bool ConsumeIf(Token::TokenType Type);
        bool Expect(Token::TokenType Type);
        [[nodiscard]] BufferStringView GetTokenLexeme(const Token& Tok) const { return TokensArena.Read(Tok.Lexeme); }
        void SendError(ParseErrorType Type, size_t Line, size_t Column, std::vector<std::string>&& Context = {});
        void SendError(ParseErrorType Type, std::vector<std::string>&& Context = {});

        [[nodiscard]] bool CanBeDataType() const;

    private:
        ASTNode* ParseSequence();
        ASTNode* ParseBlock();

        DataTypeNode* ParseDataType();

        ASTNode* ParseParameter();
        ASTNode* ParseFunction();
        ASTNode* ParseVariable();
        ASTNode* ParseIf();
        ASTNode* ParseWhile();
        ASTNode* ParseFor();
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
    };
}

#endif //CVOLT_PARSER_H