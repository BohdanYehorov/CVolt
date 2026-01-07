//
// Created by bohdan on 13.12.25.
//

#include "Lexer.h"

#include <complex.h>
#include <sstream>

#define GEN_CASE_TO_STRING(Op) case Op: TypeStr = #Op; break;

std::string Token::ToString(const ArenaStream &Stream) const
{
    std::string TypeStr;

    switch (Type)
    {
    	GEN_CASE_TO_STRING(IDENTIFIER)
    	GEN_CASE_TO_STRING(BYTE_NUMBER)
		GEN_CASE_TO_STRING(INT_NUMBER)
    	GEN_CASE_TO_STRING(LONG_NUMBER)
		GEN_CASE_TO_STRING(FLOAT_NUMBER)
    	GEN_CASE_TO_STRING(DOUBLE_NUMBER)
		GEN_CASE_TO_STRING(STRING)
		GEN_CASE_TO_STRING(BOOL_TRUE)
		GEN_CASE_TO_STRING(BOOL_FALSE)
		GEN_CASE_TO_STRING(CHAR)
		GEN_CASE_TO_STRING(OP_ADD)
		GEN_CASE_TO_STRING(OP_SUB)
		GEN_CASE_TO_STRING(OP_MUL)
		GEN_CASE_TO_STRING(OP_DIV)
		GEN_CASE_TO_STRING(OP_MOD)
		GEN_CASE_TO_STRING(OP_INC)
		GEN_CASE_TO_STRING(OP_DEC)
		GEN_CASE_TO_STRING(OP_ASSIGN)
		GEN_CASE_TO_STRING(OP_ADD_ASSIGN)
		GEN_CASE_TO_STRING(OP_SUB_ASSIGN)
		GEN_CASE_TO_STRING(OP_MUL_ASSIGN)
		GEN_CASE_TO_STRING(OP_DIV_ASSIGN)
		GEN_CASE_TO_STRING(OP_MOD_ASSIGN)
		GEN_CASE_TO_STRING(OP_AND_ASSIGN)
		GEN_CASE_TO_STRING(OP_OR_ASSIGN)
		GEN_CASE_TO_STRING(OP_XOR_ASSIGN)
		GEN_CASE_TO_STRING(OP_LSHIFT_ASSIGN)
		GEN_CASE_TO_STRING(OP_RSHIFT_ASSIGN)
		GEN_CASE_TO_STRING(OP_EQ)
		GEN_CASE_TO_STRING(OP_NEQ)
		GEN_CASE_TO_STRING(OP_GT)
		GEN_CASE_TO_STRING(OP_GTE)
		GEN_CASE_TO_STRING(OP_LT)
		GEN_CASE_TO_STRING(OP_LTE)
		GEN_CASE_TO_STRING(OP_LOGICAL_AND)
		GEN_CASE_TO_STRING(OP_LOGICAL_OR)
		GEN_CASE_TO_STRING(OP_LOGICAL_NOT)
		GEN_CASE_TO_STRING(OP_BIT_AND)
		GEN_CASE_TO_STRING(OP_BIT_OR)
		GEN_CASE_TO_STRING(OP_BIT_XOR)
		GEN_CASE_TO_STRING(OP_BIT_NOT)
		GEN_CASE_TO_STRING(OP_LSHIFT)
		GEN_CASE_TO_STRING(OP_RSHIFT)
		GEN_CASE_TO_STRING(OP_DOT)
		GEN_CASE_TO_STRING(OP_ARROW)
		GEN_CASE_TO_STRING(OP_SCOPE)
		GEN_CASE_TO_STRING(OP_QUESTION)
		GEN_CASE_TO_STRING(OP_COLON)
		GEN_CASE_TO_STRING(OP_COMMA)
		GEN_CASE_TO_STRING(OP_SEMICOLON)
		GEN_CASE_TO_STRING(OP_LPAREN)
		GEN_CASE_TO_STRING(OP_RPAREN)
		GEN_CASE_TO_STRING(OP_LBRACKET)
		GEN_CASE_TO_STRING(OP_RBRACKET)
		GEN_CASE_TO_STRING(OP_LBRACE)
		GEN_CASE_TO_STRING(OP_RBRACE)
		GEN_CASE_TO_STRING(KW_IF)
		GEN_CASE_TO_STRING(KW_ELSE)
		GEN_CASE_TO_STRING(KW_WHILE)
		GEN_CASE_TO_STRING(KW_FOR)
		GEN_CASE_TO_STRING(KW_RETURN)
		GEN_CASE_TO_STRING(KW_BREAK)
		GEN_CASE_TO_STRING(KW_CONTINUE)
		GEN_CASE_TO_STRING(TYPE_VOID)
    	GEN_CASE_TO_STRING(TYPE_BOOL)
		GEN_CASE_TO_STRING(TYPE_CHAR)
    	GEN_CASE_TO_STRING(TYPE_BYTE)
		GEN_CASE_TO_STRING(TYPE_INT)
    	GEN_CASE_TO_STRING(TYPE_LONG)
		GEN_CASE_TO_STRING(TYPE_FLOAT)
    	GEN_CASE_TO_STRING(TYPE_DOUBLE)
		GEN_CASE_TO_STRING(INVALID)
		GEN_CASE_TO_STRING(UNKNOWN)
    }

    std::stringstream SStr;

	SStr << "Token Kind: " << TypeStr << ", Lexeme: '" << Stream.Read(Lexeme).ToString() <<
		"', Pos: " << Pos << ", Line: " << Line << ", Column: " << Column;
    return SStr.str();
}

#undef GEN_CASE_TO_STRING

std::unordered_set<char> Lexer::OperatorChars = {
    '+', '-', '*', '/', '%', '=', '!', '<', '>',
    '&', '|', '^', '~', '.', ':', '?', ',', ';',
    '(', ')', '[', ']', '{', '}'
};

std::unordered_map<std::string, Token::TokenType> Lexer::Operators = {
    { "+", Token::OP_ADD },
    { "-", Token::OP_SUB },
    { "*", Token::OP_MUL },
    { "/", Token::OP_DIV },
    { "%", Token::OP_MOD },
    { "++", Token::OP_INC },
    { "--", Token::OP_DEC },

    { "=", Token::OP_ASSIGN },
    { "+=", Token::OP_ADD_ASSIGN },
    { "-=", Token::OP_SUB_ASSIGN },
    { "*=", Token::OP_MUL_ASSIGN },
    { "/=", Token::OP_DIV_ASSIGN },
    { "%=", Token::OP_MOD_ASSIGN },
    { "&=", Token::OP_AND_ASSIGN },
    { "|=", Token::OP_OR_ASSIGN },
    { "^=", Token::OP_XOR_ASSIGN },
    { "<<=", Token::OP_LSHIFT_ASSIGN },
    { ">>=", Token::OP_RSHIFT_ASSIGN },

    { "==", Token::OP_EQ },
    { "!=", Token::OP_NEQ },
    { ">", Token::OP_GT },
    { ">=", Token::OP_GTE },
    { "<", Token::OP_LT} ,
    { "<=", Token::OP_LTE },

    { "&&", Token::OP_LOGICAL_AND },
    { "||", Token::OP_LOGICAL_OR },
    { "!", Token::OP_LOGICAL_NOT },

    { "&", Token::OP_BIT_AND },
    { "|", Token::OP_BIT_OR },
    { "^", Token::OP_BIT_XOR },
    { "~", Token::OP_BIT_NOT },
    { "<<", Token::OP_LSHIFT },
    { ">>", Token::OP_RSHIFT },

    { ".", Token::OP_DOT },
    { "->", Token::OP_ARROW },
    { "::", Token::OP_SCOPE },
    { "?", Token::OP_QUESTION },
    { ":", Token::OP_COLON },
    { ",", Token::OP_COMMA },
    { ";", Token::OP_SEMICOLON },

    { "(", Token::OP_LPAREN },
    { ")", Token::OP_RPAREN },
    {"[", Token::OP_LBRACKET },
    { "]", Token::OP_RBRACKET },
    { "{", Token::OP_LBRACE },
    { "}", Token::OP_RBRACE }
};

std::unordered_map<std::string, Token::TokenType> Lexer::Keywords = {
	{ "if", Token::KW_IF },
	{ "else", Token::KW_ELSE },
	{ "while", Token::KW_WHILE },
	{ "for", Token::KW_FOR },
	{ "return", Token::KW_RETURN },
	{ "break", Token::KW_BREAK },
	{ "continue", Token::KW_CONTINUE }
};

std::unordered_map<std::string, Token::TokenType> Lexer::DataTypes = {
	{ "void", Token::TYPE_VOID },

	{ "bool", Token::TYPE_BOOL },
	{ "char", Token::TYPE_CHAR },

	{ "byte", Token::TYPE_BYTE },
	{ "int", Token::TYPE_INT },
	{ "long", Token::TYPE_LONG },

	{ "float", Token::TYPE_FLOAT },
	{ "double", Token::TYPE_DOUBLE }
};

std::string Lexer::GetOperatorLexeme(Token::TokenType Type)
{
	static std::unordered_map<Token::TokenType, std::string> ReversedOperatorsMap;
	if (ReversedOperatorsMap.empty())
	{
		for (const auto& [Lexeme, TokenType] : Operators)
			ReversedOperatorsMap[TokenType] = Lexeme;
	}

	if (auto Iter = ReversedOperatorsMap.find(Type); Iter != ReversedOperatorsMap.end())
		return Iter->second;
	return "";
}

void Lexer::Lex()
{
    Token Tok;
    while (IsValidPos())
    {
        SkipSpaces();

		if (GetIdentifierToken(Tok))
			Tokens.push_back(Tok);
		else if (GetNumberToken(Tok))
			Tokens.push_back(Tok);
		else if (GetOperatorToken(Tok))
			Tokens.push_back(Tok);
		else if (GetStringToken(Tok))
    		Tokens.push_back(Tok);
		else if (GetChar(Tok))
    		Tokens.push_back(Tok);
		else
        {
        	SendError(LexErrorType::InvalidCharacter, Line, Column, { std::string(CurrentChar(), 1) });
	        MovePos();
        }
    }

	TokensArena.SetAutoReallocate(false);
}

void Lexer::PrintTokens() const
{
	for (const Token& Tok : Tokens)
		std::cout << Tok.ToString(TokensArena) << std::endl;
}

void Lexer::MovePos()
{
	if (CurrentChar() == '\n')
	{
		Line++;
		Column = 1;
	}
	else
		Column++;

	Pos++;
}

void Lexer::MovePos(size_t Chars)
{
	while (Chars--)
		MovePos();
}

void Lexer::SkipSpaces()
{
    while (IsValidPos() && std::isspace(CurrentUChar()))
        MovePos();
}

bool Lexer::GetIdentifierToken(Token &Tok)
{
    size_t StartPos = Pos, StartLine = Line, StartCol = Column;

    if (uchar Ch = CurrentUChar(); std::isalpha(Ch) || Ch == '_')
        MovePos();
    else
        return false;

    while (IsValidPos())
    {
        if (uchar Ch = CurrentUChar(); std::isalpha(Ch) || Ch == '_' || std::isdigit(Ch))
        {
            MovePos();
            continue;
        }

        break;
    }

    if (StartPos == Pos)
        return false;

	StringRef Lexeme(ExprRef.Ptr + StartPos, Pos - StartPos);
	Token::TokenType TokenType = Token::IDENTIFIER;

	BufferStringView View = TokensArena.Read(Lexeme);
	if (auto KwIter = Keywords.find(View.ToString()); KwIter != Keywords.end())
		TokenType = KwIter->second;
	else if (auto TypeIter = DataTypes.find(View.ToString()); TypeIter != DataTypes.end())
		TokenType = TypeIter->second;
	else if (View == "true")
		TokenType = Token::BOOL_TRUE;
	else if (View == "false")
		TokenType = Token::BOOL_FALSE;

    Tok = Token(
        TokenType, Lexeme,
        StartPos, StartLine, StartCol);
    return true;
}

bool Lexer::GetNumberToken(Token &Tok)
{

	if (CurrentChar() == '.')
	{
		if (IsValidNextPos())
		{
			uchar Ch = NextUChar();
			if (!isdigit(Ch))
			{
				Tok = Token(Token::OP_DOT, { Pos, 1 }, Pos, Line, Column);
				MovePos();
				return true;
			}
		}
		else
		{
			Tok = Token(Token::OP_DOT, { Pos, 1 }, Pos, Line, Column);
			MovePos();
			return true;
		}
	}

    size_t StartPos = Pos, StartLine = Line, StartCol = Column;

    bool HasDigit = false;
    bool HasDot = false;
    bool HasExponent = false;
    bool HasExponentSign = false;
    bool HasExponentDigits = false;
    bool IsInvalidToken = false;

	StringRef Suffix{ 0, 0 };

    while (IsValidPos())
    {
        uchar Ch = CurrentUChar();

        if (isdigit(Ch))
        {
            HasDigit = true;
            if (HasExponent)
                HasExponentDigits = true;
        }
        else if (Ch == '.')
        {
            if (HasDot || HasExponent)
            {
            	if (!IsInvalidToken)
	            {
		            IsInvalidToken = true;
	            	SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
	            }
            }
            else
                HasDot = true;
        }
        else if (std::tolower(Ch) == 'e')
        {
            if (!HasDigit || HasExponent)
            {
            	if (!IsInvalidToken)
            	{
            		IsInvalidToken = true;
            		SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
            	}
            }
            else
                HasExponent = true;
        }
        else if (Ch == '-' || Ch == '+')
        {
            if (!HasExponent)
                break;

            if (HasExponentDigits || HasExponentSign)
            {
            	if (!IsInvalidToken)
            	{
            		IsInvalidToken = true;
            		SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
            	}
            }
            else
                HasExponentSign = true;
        }
        else if (isalpha(Ch) || Ch == '_')
        {
        	// if (!InvalidToken)
        	// {
        	// 	InvalidToken = true;
        	// 	SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
        	// }

        	if (Suffix.Length == 0)
        		Suffix.Ptr = ExprRef.Ptr + Pos;

        	Suffix.Length++;
        }
        else
            break;

        MovePos();
    }

    if (StartPos == Pos)
        return false;

	if (HasExponent && !HasExponentDigits)
	{
		if (!IsInvalidToken)
		{
			IsInvalidToken = true;
			SendError(LexErrorType::UnterminatedNumber, StartLine, StartCol);
		}
	}

    StringRef Lexeme(ExprRef.Ptr + StartPos, Pos - StartPos);
	Token::TokenType TokenType = Token::INT_NUMBER;

	BufferStringView SuffixStr = TokensArena.Read(Suffix);

	if (IsInvalidToken)
	{
		Tok = Token(Token::INVALID, Lexeme,
					StartPos, StartLine, StartCol);

		return true;
	}

	if (HasDot || HasExponent)
	{
		if (Suffix.Length == 0)
			TokenType = Token::DOUBLE_NUMBER;
		else if (SuffixStr == "f")
			TokenType = Token::FLOAT_NUMBER;
		else
		{
			SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
			Tok = InvalidToken(StartPos, StartLine, StartCol);
			return true;
		}
	}
	else
	{
		if (Suffix.Length == 0)
			TokenType = Token::INT_NUMBER;
		else if (SuffixStr == "b")
			TokenType = Token::BYTE_NUMBER;
		else if (SuffixStr == "l")
			TokenType = Token::LONG_NUMBER;
		else
		{
			SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
			Tok = InvalidToken(StartPos, StartLine, StartCol);
			return true;
		}
	}

	Tok = Token(TokenType, Lexeme,
					StartPos, StartLine, StartCol);

    return true;
}

bool Lexer::GetOperatorToken(Token &Tok)
{
	if (!OperatorChars.contains(CurrentChar()))
		return false;

    static size_t MaxOperatorSize = 0;
    if (MaxOperatorSize == 0)
    {
        for (const auto& [Op, Type] : Operators)
        {
            size_t OpSize = Op.size();
            if (MaxOperatorSize < OpSize)
                MaxOperatorSize = OpSize;
        }
    }

    size_t StartPos = Pos, StartLine = Line, StartCol = Column;

	size_t Len = std::min(MaxOperatorSize, ExprRef.Length - StartPos);
    StringRef OperatorLexeme(ExprRef.Ptr + Pos, Len);

    while (Len > 0)
    {
        OperatorLexeme.Length = Len;

        if (auto Iter = Operators.find(TokensArena.Read(OperatorLexeme).ToString()); Iter != Operators.end())
        {
            Tok = Token(Iter->second, OperatorLexeme, StartPos, StartLine, StartCol);
        	MovePos(Len);
            return true;
        }
    	Len--;
    }

    return false;
}

bool Lexer::GetChar(Token &Tok)
{
	if (CurrentChar() != '\'')
		return false;

	size_t StartPos = Pos, StartLine = Line, StartCol = Column;
	MovePos();

	if (!IsValidPos())
	{
		SendError(LexErrorType::UnexpectedEOF, StartLine, StartCol);
		Tok = InvalidToken(StartPos, StartLine, StartLine);
		return true;
	}

	char Ch = CurrentChar();

	MovePos();

	if (!IsValidPos())
	{
		SendError(LexErrorType::UnexpectedEOF, StartLine, StartCol);
		Tok = InvalidToken(StartPos, StartLine, StartLine);
		return true;
	}

	if (Ch == '\\')
	{
		GetEscape(CurrentChar(), Ch);
		MovePos();
	}

	if (!IsValidPos())
	{
		SendError(LexErrorType::UnexpectedEOF, StartLine, StartCol);
		Tok = InvalidToken(StartPos, StartLine, StartLine);
		return true;
	}

	if (CurrentChar() != '\'')
	{
		SendError(LexErrorType::UnterminatedCharacterLiteral, StartLine, StartCol);
		Tok = InvalidToken(StartPos, StartLine, StartLine);
		return true;
	}

	MovePos();

	Tok = Token(Token::CHAR, TokensArena.Write(std::string(1, Ch)), StartPos, StartLine, StartCol);
	return true;
}

bool Lexer::GetStringToken(Token &Tok)
{
	if (CurrentChar() != '"')
		return false;

	size_t StartPos = Pos, StartLine = Line, StartCol = Column;
	MovePos();

	std::string Str;

	while (IsValidPos())
	{
		char Ch = CurrentChar();
		if (Ch == '"')
		{
			MovePos();
			TokensArena.Write(Str);
			Tok = Token(Token::STRING, TokensArena.Write(Str), StartPos, StartLine, StartCol);
			return true;
		}
		if (Ch == '\\')
		{
			MovePos();
			if (!IsValidPos())
			{
				SendError(LexErrorType::UnterminatedEscape, StartLine, StartCol);
				Tok = Token(Token::INVALID, TokensArena.Write(Str), StartPos, StartLine, StartCol);
				return true;
			}

			if (char Escape; GetEscape(CurrentChar(), Escape))
				Str.push_back(Escape);
		}
		else if (Ch == '\n')
		{
			SendError(LexErrorType::NewlineInString, StartLine, StartCol);
			Tok = InvalidToken(TokensArena.Write(Str), StartPos, StartLine, StartCol);
			return true;
		}
		else
			Str.push_back(Ch);

		MovePos();
	}

	SendError(LexErrorType::UnterminatedString, StartLine, StartCol);
	Tok = Token(Token::INVALID, TokensArena.Write(Str), StartPos, StartLine, StartCol);
	return true;
}

bool Lexer::GetEscape(char Ch, char& Escape)
{
	switch (Ch)
	{
		case '\'':
			Escape = '\'';
			break;
		case '"':
			Escape = '"';
			break;
		case '\\':
			Escape = '\\';
			break;
		case '?':
			Escape = '\?';
			break;
		case 'a':
			Escape = '\a';
			break;
		case 'b':
			Escape = '\b';
			break;
		case 'f':
			Escape = '\f';
			break;
		case 'n':
			Escape = '\n';
			break;
		case 'r':
			Escape = '\r';
			break;
		case 't':
			Escape = '\t';
			break;
		case 'v':
			Escape = '\v';
			break;
		default:
			SendError(LexErrorType::InvalidEscape, Line, Column, { std::string(Escape, 1) });
			return false;
	}

	return true;
}