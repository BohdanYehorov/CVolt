//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_PARSER_H
#define CVOLT_PARSER_H

#include "Lexer.h"
#include "ASTNodes.h"
#include "Arena.h"
#include "Errors.h"

class Parser
{
private:
    Arena NodesArena;
    const ArenaStream& TokensArena;

    const std::vector<Token>& Tokens;
    std::vector<ParseError> ErrorList;

    size_t Index = 0;
    ASTNode* Root = nullptr;

    bool LastNodeIsBlock = false;
    bool InFunction = false;
    bool InLoop = false;
    size_t LoopsCount = 0;

private:
    static void PrintASTTree(ASTNode* Node, int Tabs = 0);

public:
    Parser(const Lexer& L) : TokensArena(L.GetTokensArena()), Tokens(L.GetTokens()) {}

    void Parse();
    [[nodiscard]] ASTNode* GetASTTree() const { return Root; }
    [[nodiscard]] const std::vector<ParseError>& GetErrorList() const { return ErrorList; }
    void PrintASTTree() const;

private:
    [[nodiscard]] bool IsValidIndex() const { return Index < Tokens.size(); }
    [[nodiscard]] const Token& CurrentToken() const { return Tokens[Index]; }
    bool Consume();
    void SkipSemicolons();
    void Synchronize();
    bool GetTokenIf(size_t Index, Token::TokenType Type, const Token*& TokPtr) const;
    bool GetNextTokenIf(Token::TokenType Type, const Token*& TokPtr, size_t NextIndexOffset = 1) const;
    bool Peek(Token::TokenType Type, const Token*& TokPtr) const;
    [[nodiscard]] bool Peek(Token::TokenType Type) const;
    bool ConsumeIf(Token::TokenType Type, const Token*& TokPtr);
    bool ConsumeIf(Token::TokenType Type);
    bool Expect(Token::TokenType Type);
    [[nodiscard]] BufferStringView GetTokenLexeme(const Token& Tok) const { return TokensArena.Read(Tok.Lexeme); }

private:
    ASTNode* ParseSequence();
    ASTNode* ParseBlock();

    DataTypeNodeBase* ParseDataType();

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

#endif //CVOLT_PARSER_H