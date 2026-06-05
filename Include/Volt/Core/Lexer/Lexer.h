//
// Created by bohdan on 13.12.25.
//

#ifndef CVOLT_LEXER_H
#define CVOLT_LEXER_H

#include "Volt/Core/Errors/LexError.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/ADT/Array.h"
#include <llvm/ADT/StringMap.h>
#include "Token.h"
#include <string>

namespace Volt
{
    using UChar = unsigned char;

    class Lexer
    {
    private:
        static llvm::StringMap<TokenType> Operators;
        static llvm::StringMap<TokenType> Keywords;
        static llvm::StringMap<TokenType> DataTypes;
        static llvm::StringMap<TokenType> IntNumberLiterals;
        static llvm::StringMap<TokenType> FloatNumberLiterals;

    public:
        static std::string GetOperatorLexeme(TokenType Type);

        static bool IsOperatorChar(UChar Ch);

    private:
        size_t Pos = 0, Line = 1, Column = 1;

        ArenaStream TokensArena;

        CompilationContext& Context;
        String& Code;

        size_t CodeSize;

        Array<Token>& Tokens;
        Array<LexError>& Errors;

    public:
        Lexer(CompilationContext& Context)
            : Context(Context), Code(Context.Code), CodeSize(Code.Length()),
            Tokens(Context.Tokens), Errors(Context.LexErrors) {}

        Lexer(const Lexer&) = delete;
        Lexer& operator=(const Lexer&) = delete;

        Lexer(Lexer&&) noexcept = delete;
        Lexer& operator=(Lexer&&) noexcept = delete;

        void Lex();

        [[nodiscard]] const ArenaStream& GetTokensArena() const { return TokensArena; }

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

        bool GetNumberSuffixLiteral(llvm::StringRef& Lit);

        [[nodiscard]] static Token InvalidToken(StringRef Lexeme, size_t Pos, size_t Line, size_t Col)
        { return { TokenType::INVALID, Lexeme, Pos, Line, Col };  }

        [[nodiscard]] Token InvalidToken(size_t StartPos, size_t StartLine, size_t StartCol) const
        { return InvalidToken(StringRef(StartPos, Pos - StartPos),
            StartPos, StartLine, StartCol); }

        void SendError(LexErrorType Type, size_t ErrLine, size_t ErrColumn, Array<std::string>&& Ctx = {})
                { Errors.Emplace(Type, ErrLine, ErrColumn, std::move(Ctx)); }
    };
}
#endif //CVOLT_LEXER_H