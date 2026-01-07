//
// Created by bohdan on 13.12.25.
//

#ifndef CVOLT_LEXER_H
#define CVOLT_LEXER_H

#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "Errors.h"
#include "Arena.h"
#include <iostream>

using uchar = unsigned char;

struct Token
{
    enum TokenType
    {
        IDENTIFIER,

        BYTE_NUMBER,
        INT_NUMBER,
        LONG_NUMBER,

        FLOAT_NUMBER,
        DOUBLE_NUMBER,

        STRING,
        BOOL_TRUE,
        BOOL_FALSE,
        CHAR,

        OP_ADD,
        OP_SUB,
        OP_MUL,
        OP_DIV,
        OP_MOD,
        OP_INC,
        OP_DEC,

        OP_ASSIGN,
        OP_ADD_ASSIGN,
        OP_SUB_ASSIGN,
        OP_MUL_ASSIGN,
        OP_DIV_ASSIGN,
        OP_MOD_ASSIGN,
        OP_AND_ASSIGN,
        OP_OR_ASSIGN,
        OP_XOR_ASSIGN,
        OP_LSHIFT_ASSIGN,
        OP_RSHIFT_ASSIGN,

        OP_EQ,
        OP_NEQ,
        OP_GT,
        OP_GTE,
        OP_LT,
        OP_LTE,

        OP_LOGICAL_AND,
        OP_LOGICAL_OR,
        OP_LOGICAL_NOT,

        OP_BIT_AND,
        OP_BIT_OR,
        OP_BIT_XOR,
        OP_BIT_NOT,
        OP_LSHIFT,
        OP_RSHIFT,

        OP_DOT,
        OP_ARROW,
        OP_SCOPE,
        OP_QUESTION,
        OP_COLON,
        OP_COMMA,
        OP_SEMICOLON,

        OP_LPAREN,
        OP_RPAREN,
        OP_LBRACKET,
        OP_RBRACKET,
        OP_LBRACE,
        OP_RBRACE,

        KW_IF,
        KW_ELSE,
        KW_WHILE,
        KW_FOR,
        KW_RETURN,
        KW_BREAK,
        KW_CONTINUE,

        TYPE_VOID,

        TYPE_BOOL,
        TYPE_CHAR,

        TYPE_BYTE,
        TYPE_INT,
        TYPE_LONG,

        TYPE_FLOAT,
        TYPE_DOUBLE,

        INVALID,
        UNKNOWN
    };

    TokenType Type = UNKNOWN;
    StringRef Lexeme;
    size_t Pos = 0, Line = 1, Column = 1;

    Token() = default;
    Token(TokenType Type, StringRef Lexeme, size_t Pos, size_t Line, size_t Column)
        : Type(Type), Lexeme(Lexeme), Pos(Pos), Line(Line), Column(Column) {}

    [[nodiscard]] std::string ToString(const ArenaStream &Stream) const;
};

class Lexer
{
private:
    static std::unordered_set<char> OperatorChars;
    static std::unordered_map<std::string, Token::TokenType> Operators;
    static std::unordered_map<std::string, Token::TokenType> Keywords;
    static std::unordered_map<std::string, Token::TokenType> DataTypes;

public:
    static std::string GetOperatorLexeme(Token::TokenType Type);

private:
    size_t Pos = 0, Line = 1, Column = 1;
    std::vector<Token> Tokens;
    std::vector<LexError> Errors;
    std::vector<char> CharStorage;

    ArenaStream TokensArena;
    StringRef ExprRef;
    PtrT StringStoragePtr = 0;

public:
    Lexer() = default;
    Lexer(const std::string& Expr)
    {
        TokensArena.SetAutoReallocate(true);
        ExprRef = TokensArena.Write(Expr);
        StringStoragePtr = TokensArena.GetWritePtr();
        std::cout << TokensArena.GetWritePtr() << std::endl;
    }
    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    Lexer(Lexer&&) noexcept = default;
    Lexer& operator=(Lexer&&) noexcept = default;

    void Lex();
    void PrintTokens() const;

    [[nodiscard]] const std::vector<Token>& GetTokens() const { return Tokens; }
    [[nodiscard]] const ArenaStream& GetTokensArena() const { return TokensArena; }
    std::vector<LexError> GetErrors() { return Errors; }
    [[nodiscard]] bool HasErrors() const { return !Errors.empty(); }

private:
    [[nodiscard]] char CurrentChar() const { return *TokensArena.GetArenaAllocator().Read<char>(Pos); }
    [[nodiscard]] uchar CurrentUChar() const { return static_cast<uchar>(CurrentChar()); }
    [[nodiscard]] uchar NextUChar() const { return static_cast<uchar>(*TokensArena.GetArenaAllocator().Read<char>(Pos + 1)); }
    [[nodiscard]] bool IsValidPos() const { return Pos < ExprRef.Length; }
    [[nodiscard]] bool IsValidNextPos() const { return Pos + 1 < ExprRef.Length; }

    void MovePos();
    void MovePos(size_t Chars);
    void SkipSpaces();

    bool GetIdentifierToken(Token& Tok);
    bool GetNumberToken(Token& Tok);
    bool GetOperatorToken(Token& Tok);
    bool GetChar(Token& Tok);
    bool GetStringToken(Token& Tok);

    bool GetEscape(char Ch, char& Escape);

    [[nodiscard]] static Token InvalidToken(StringRef Lexeme, size_t Pos, size_t Line, size_t Col)
    { return { Token::INVALID, Lexeme, Pos, Line, Col };  }

    [[nodiscard]] Token InvalidToken(size_t StartPos, size_t StartLine, size_t StartCol) const
    { return InvalidToken(StringRef(ExprRef.Ptr + StartPos, Pos - StartPos),
        StartPos, StartLine, StartCol); }

    void SendError(LexErrorType Type, size_t ErrLine, size_t ErrColumn, std::vector<std::string>&& Context = {})
    { Errors.emplace_back(Type, ErrLine, ErrColumn, std::move(Context)); }
};

#endif //CVOLT_LEXER_H