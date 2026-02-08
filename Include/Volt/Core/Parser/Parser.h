//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_PARSER_H
#define CVOLT_PARSER_H

#include "Volt/Core/Lexer/Lexer.h"
#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/Core/Errors/ParseError.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
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
        CompilationContext& CContext;
        Arena& NodesArena;

        const Array<Token>& Tokens;
        Array<ParseError> Errors;

        size_t Index = 0;
        ASTNode*& Root;

        bool LastNodeIsBlock = false;
        bool InBlock = false;
        bool InFunction = false;
        bool InLoop = false;

        size_t Depth = 0;

    private:
        static void WriteASTTree(std::ostream& Os, ASTNode* Node, int Tabs = 0);

    public:
        Parser(CompilationContext& CContext)
            : CContext(CContext), NodesArena(CContext.MainArena),
            Tokens(CContext.Tokens), Root(CContext.ASTTree) {}

        void Parse();
        [[nodiscard]] ASTNode* GetASTTree() const { return Root; }
        [[nodiscard]] const Array<ParseError>& GetErrorList() const { return Errors; }
        [[nodiscard]] bool HasErrors() const { return !Errors.Empty(); }
        bool PrintErrors() const
        {
            WriteErrors(std::cout);
            return HasErrors();
        }

        void WriteErrors(std::ostream& Os) const;

        void WriteASTTree(std::ostream& Os) const { WriteASTTree(Os, Root); }
        void PrintASTTree() const { WriteASTTree(std::cout); };

    private:
        [[nodiscard]] bool IsValidIndex() const { return Index < Tokens.Length(); }
        [[nodiscard]] const Token& CurrentToken() const { return Tokens[Index]; }
        [[nodiscard]] const Token& PrevToken() const { return Tokens[Index - 1]; }
        bool Consume();
        void SkipSemicolons();
        void Synchronize();
        void JumpToNextGlobalDeclaration();
        bool GetTokenIf(size_t Index, TokenType Type, const Token*& TokPtr) const;
        bool GetNextTokenIf(TokenType Type, const Token*& TokPtr, size_t NextIndexOffset = 1) const;
        bool Peek(TokenType Type, const Token*& TokPtr) const;
        [[nodiscard]] bool Peek(TokenType Type) const;
        bool ConsumeIf(TokenType Type, const Token*& TokPtr);
        bool ConsumeIf(TokenType Type);
        bool Expect(TokenType Type);

        [[nodiscard]] llvm::StringRef GetTokenLexeme(const Token& Tok) const
        {
            return CContext.GetTokenLexeme(Tok.Lexeme);
        }

        void SendError(ParseErrorType Type, size_t Line, size_t Column, Array<std::string>&& Context = {});
        void SendError(ParseErrorType Type, Array<std::string>&& Context = {});

        [[nodiscard]] bool CanBeDataType() const;

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
}

#endif //CVOLT_PARSER_H