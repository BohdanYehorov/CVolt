//
// Created by bohdan on 13.12.25.
//

#ifndef CVOLT_LEXER_H
#define CVOLT_LEXER_H

#include "Volt/Core/Errors/LexError.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Token.h"
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace Volt
{
    using UChar = unsigned char;

    class Lexer
    {
    private:
        static std::unordered_set<char> OperatorChars;
        static std::unordered_map<std::string, TokenType> Operators;
        static std::unordered_map<std::string, TokenType> Keywords;
        static std::unordered_map<std::string, TokenType> DataTypes;

    public:
        static std::string GetOperatorLexeme(TokenType Type);

    private:
        size_t Pos = 0, Line = 1, Column = 1;
        std::vector<LexError> Errors;
        std::vector<char> CharStorage;

        ArenaStream TokensArena;
        StringRef ExprRef;
        PtrT StringStoragePtr = 0;

        CompilationContext& Context;
        std::string& Code;

        size_t CodeSize;

        std::vector<Token>& Tokens;

    public:
        //Lexer(const std::string& Expr);
        Lexer(CompilationContext& Context);

        Lexer(const Lexer&) = delete;
        Lexer& operator=(const Lexer&) = delete;

        Lexer(Lexer&&) noexcept = delete;
        Lexer& operator=(Lexer&&) noexcept = delete;

        void Lex();
        void PrintTokens() const { WriteTokens(std::cout); }

        [[nodiscard]] const std::vector<Token>& GetTokens() const { return Tokens; }
        [[nodiscard]] const ArenaStream& GetTokensArena() const { return TokensArena; }
        std::vector<LexError> GetErrors() { return Errors; }
        [[nodiscard]] bool HasErrors() const { return !Errors.empty(); }
        bool PrintErrors() const
        {
            WriteErrors(std::cout);
            return HasErrors();
        }
        void WriteErrors(std::ostream& Os) const;
        void WriteTokens(std::ostream& Os) const;

    private:
        [[nodiscard]] char CurrentChar() const { return Code[Pos]; }
        [[nodiscard]] UChar CurrentUChar() const { return static_cast<UChar>(Code[Pos]); }
        [[nodiscard]] char NextChar() const { return Code[Pos + 1]; }
        [[nodiscard]] UChar NextUChar() const { return static_cast<UChar>(Code[Pos]); }
        [[nodiscard]] bool IsValidPos() const { return Pos < CodeSize; }
        [[nodiscard]] bool IsValidNextPos() const { return Pos + 1 < CodeSize; }

        void MovePos();
        void MovePos(size_t Chars);
        void SkipSpaces();
        void SkipComments();

        bool GetIdentifierToken(Token& Tok);
        bool GetNumberToken(Token& Tok);
        bool GetOperatorToken(Token& Tok);
        bool GetChar(Token& Tok);
        bool GetStringToken(Token& Tok);

        bool GetEscape(char Ch, char& Escape);

        [[nodiscard]] static Token InvalidToken(StringRef Lexeme, size_t Pos, size_t Line, size_t Col)
        { return { TokenType::INVALID, Lexeme, Pos, Line, Col };  }

        [[nodiscard]] Token InvalidToken(size_t StartPos, size_t StartLine, size_t StartCol) const
        { return InvalidToken(StringRef(StartPos, Pos - StartPos),
            StartPos, StartLine, StartCol); }

        void SendError(LexErrorType Type, size_t ErrLine, size_t ErrColumn,
            std::vector<std::string>&& Context = {})
                { Errors.emplace_back(Type, ErrLine, ErrColumn, std::move(Context)); }
    };
}
#endif //CVOLT_LEXER_H