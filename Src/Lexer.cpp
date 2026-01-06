//
// Created by bohdan on 13.12.25.
//

#include "Lexer.h"
#include <sstream>

std::string Token::ToString(const ArenaStream &Stream) const
{
    std::string TypeStr;

    switch (Type)
    {
    	case IDENTIFIER:
			TypeStr = "IDENTIFIER";
			break;
		case INT_NUMBER:
			TypeStr = "INT_NUMBER";
			break;
		case FLOAT_NUMBER:
			TypeStr = "FLOAT_NUMBER";
			break;
		case STRING:
			TypeStr = "STRING";
			break;
		case BOOL_TRUE:
			TypeStr = "BOOL_TRUE";
			break;
		case BOOL_FALSE:
			TypeStr = "BOOL_FALSE";
			break;
		case CHAR:
			TypeStr = "CHAR";
			break;
		case OP_ADD:
			TypeStr = "OP_ADD";
			break;
		case OP_SUB:
			TypeStr = "OP_SUB";
			break;
		case OP_MUL:
			TypeStr = "OP_MUL";
			break;
		case OP_DIV:
			TypeStr = "OP_DIV";
			break;
		case OP_MOD:
			TypeStr = "OP_MOD";
			break;
		case OP_INC:
			TypeStr = "OP_INC";
			break;
		case OP_DEC:
			TypeStr = "OP_DEC";
			break;
		case OP_ASSIGN:
			TypeStr = "OP_ASSIGN";
			break;
		case OP_ADD_ASSIGN:
			TypeStr = "OP_ADD_ASSIGN";
			break;
		case OP_SUB_ASSIGN:
			TypeStr = "OP_SUB_ASSIGN";
			break;
		case OP_MUL_ASSIGN:
			TypeStr = "OP_MUL_ASSIGN";
			break;
		case OP_DIV_ASSIGN:
			TypeStr = "OP_DIV_ASSIGN";
			break;
		case OP_MOD_ASSIGN:
			TypeStr = "OP_MOD_ASSIGN";
			break;
		case OP_AND_ASSIGN:
			TypeStr = "OP_AND_ASSIGN";
			break;
		case OP_OR_ASSIGN:
			TypeStr = "OP_OR_ASSIGN";
			break;
		case OP_XOR_ASSIGN:
			TypeStr = "OP_XOR_ASSIGN";
			break;
		case OP_LSHIFT_ASSIGN:
			TypeStr = "OP_LSHIFT_ASSIGN";
			break;
		case OP_RSHIFT_ASSIGN:
			TypeStr = "OP_RSHIFT_ASSIGN";
			break;
		case OP_EQ:
			TypeStr = "OP_EQ";
			break;
		case OP_NEQ:
			TypeStr = "OP_NEQ";
			break;
		case OP_GT:
			TypeStr = "OP_GT";
			break;
		case OP_GTE:
			TypeStr = "OP_GTE";
			break;
		case OP_LT:
			TypeStr = "OP_LT";
			break;
		case OP_LTE:
			TypeStr = "OP_LTE";
			break;
		case OP_LOGICAL_AND:
			TypeStr = "OP_LOGICAL_AND";
			break;
		case OP_LOGICAL_OR:
			TypeStr = "OP_LOGICAL_OR";
			break;
		case OP_LOGICAL_NOT:
			TypeStr = "OP_LOGICAL_NOT";
			break;
		case OP_BIT_AND:
			TypeStr = "OP_BIT_AND";
			break;
		case OP_BIT_OR:
			TypeStr = "OP_BIT_OR";
			break;
		case OP_BIT_XOR:
			TypeStr = "OP_BIT_XOR";
			break;
		case OP_BIT_NOT:
			TypeStr = "OP_BIT_NOT";
			break;
		case OP_LSHIFT:
			TypeStr = "OP_LSHIFT";
			break;
		case OP_RSHIFT:
			TypeStr = "OP_RSHIFT";
			break;
		case OP_DOT:
			TypeStr = "OP_DOT";
			break;
		case OP_ARROW:
			TypeStr = "OP_ARROW";
			break;
		case OP_SCOPE:
			TypeStr = "OP_SCOPE";
			break;
		case OP_QUESTION:
			TypeStr = "OP_QUESTION";
			break;
		case OP_COLON:
			TypeStr = "OP_COLON";
			break;
		case OP_COMMA:
			TypeStr = "OP_COMMA";
			break;
		case OP_SEMICOLON:
			TypeStr = "OP_SEMICOLON";
			break;
		case OP_LPAREN:
			TypeStr = "OP_LPAREN";
			break;
		case OP_RPAREN:
			TypeStr = "OP_RPAREN";
			break;
		case OP_LBRACKET:
			TypeStr = "OP_LBRACKET";
			break;
		case OP_RBRACKET:
			TypeStr = "OP_RBRACKET";
			break;
		case OP_LBRACE:
			TypeStr = "OP_LBRACE";
			break;
		case OP_RBRACE:
			TypeStr = "OP_RBRACE";
			break;
		case KW_IF:
			TypeStr = "KW_IF";
			break;
		case KW_ELSE:
			TypeStr = "KW_ELSE";
			break;
		case KW_WHILE:
			TypeStr = "KW_WHILE";
			break;
		case KW_FOR:
			TypeStr = "KW_FOR";
			break;
		case KW_RETURN:
			TypeStr = "KW_RETURN";
			break;
		case KW_BREAK:
			TypeStr = "KW_BREAK";
			break;
		case KW_CONTINUE:
			TypeStr = "KW_CONTINUE";
			break;
    	case TYPE_VOID:
    		TypeStr = "TYPE_VOID";
    		break;
		case TYPE_INT:
			TypeStr = "TYPE_INT";
			break;
		case TYPE_FLOAT:
			TypeStr = "TYPE_FLOAT";
			break;
		case TYPE_BOOL:
			TypeStr = "TYPE_BOOL";
			break;
		case TYPE_CHAR:
			TypeStr = "TYPE_CHAR";
			break;
		case INVALID:
			TypeStr = "INVALID";
			break;
		case UNKNOWN:
			TypeStr = "UNKNOWN";
			break;
    }

    std::stringstream SStr;

	SStr << TypeStr << ", " << Stream.Read(Lexeme).ToString() << ", " << Pos << ", " << Line << ", " << Column;
    return SStr.str();
}

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
	{ "int", Token::TYPE_INT },
	{ "float", Token::TYPE_FLOAT },
	{ "bool", Token::TYPE_BOOL },
	{ "char", Token::TYPE_CHAR }
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
    bool InvalidToken = false;

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
            	if (!InvalidToken)
	            {
		            InvalidToken = true;
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
            	if (!InvalidToken)
            	{
            		InvalidToken = true;
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
            	if (!InvalidToken)
            	{
            		InvalidToken = true;
            		SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
            	}
            }
            else
                HasExponentSign = true;
        }
        else if (isalpha(Ch) || Ch == '_')
        {
        	if (!InvalidToken)
        	{
        		InvalidToken = true;
        		SendError(LexErrorType::InvalidNumber, StartLine, StartCol);
        	}
        }
        else
            break;

        MovePos();
    }

    if (StartPos == Pos)
        return false;

	if (HasExponent && !HasExponentDigits)
	{
		if (!InvalidToken)
		{
			InvalidToken = true;
			SendError(LexErrorType::UnterminatedNumber, StartLine, StartCol);
		}
	}

    StringRef Lexeme(ExprRef.Ptr + StartPos, Pos - StartPos);
	Token::TokenType TokenType = Token::INT_NUMBER;

    if (InvalidToken)
        TokenType = Token::INVALID;
    else if (HasDot || HasExponent)
    	TokenType = Token::FLOAT_NUMBER;

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

	size_t Index = CharStorage.size();
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