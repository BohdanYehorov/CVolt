//
// Created by bohdan on 13.12.25.
//

#include "Volt/Core/Lexer/Lexer.h"

#include <complex>
#include <sstream>

namespace Volt
{
	std::unordered_set<char> Lexer::OperatorChars = {
		'+', '-', '*', '/', '%', '=', '!', '<', '>',
		'&', '|', '^', '~', '.', ':', '?', ',', ';',
		'(', ')', '[', ']', '{', '}', '$'
	};

	std::unordered_map<std::string, TokenType> Lexer::Operators = {
		{ "+", TokenType::OP_ADD },
		{ "-", TokenType::OP_SUB },
		{ "*", TokenType::OP_MUL },
		{ "/", TokenType::OP_DIV },
		{ "%", TokenType::OP_MOD },
		{ "++", TokenType::OP_INC },
		{ "--", TokenType::OP_DEC },

		{ "=", TokenType::OP_ASSIGN },
		{ "+=", TokenType::OP_ADD_ASSIGN },
		{ "-=", TokenType::OP_SUB_ASSIGN },
		{ "*=", TokenType::OP_MUL_ASSIGN },
		{ "/=", TokenType::OP_DIV_ASSIGN },
		{ "%=", TokenType::OP_MOD_ASSIGN },
		{ "&=", TokenType::OP_AND_ASSIGN },
		{ "|=", TokenType::OP_OR_ASSIGN },
		{ "^=", TokenType::OP_XOR_ASSIGN },
		{ "<<=", TokenType::OP_LSHIFT_ASSIGN },
		{ ">>=", TokenType::OP_RSHIFT_ASSIGN },

		{ "==", TokenType::OP_EQ },
		{ "!=", TokenType::OP_NEQ },
		{ ">", TokenType::OP_GT },
		{ ">=", TokenType::OP_GTE },
		{ "<", TokenType::OP_LT} ,
		{ "<=", TokenType::OP_LTE },

		{ "&&", TokenType::OP_LOGICAL_AND },
		{ "||", TokenType::OP_LOGICAL_OR },
		{ "!", TokenType::OP_LOGICAL_NOT },

		{ "&", TokenType::OP_BIT_AND },
		{ "|", TokenType::OP_BIT_OR },
		{ "^", TokenType::OP_BIT_XOR },
		{ "~", TokenType::OP_BIT_NOT },
		{ "<<", TokenType::OP_LSHIFT },
		{ ">>", TokenType::OP_RSHIFT },

		{ ".", TokenType::OP_DOT },
		{ "->", TokenType::OP_ARROW },
		{ "::", TokenType::OP_SCOPE },
		{ "?", TokenType::OP_QUESTION },
		{ ":", TokenType::OP_COLON },
		{ ",", TokenType::OP_COMMA },
		{ ";", TokenType::OP_SEMICOLON },

		{ "(", TokenType::OP_LPAREN },
		{ ")", TokenType::OP_RPAREN },
		{"[", TokenType::OP_LBRACKET },
		{ "]", TokenType::OP_RBRACKET },
		{ "{", TokenType::OP_LBRACE },
		{ "}", TokenType::OP_RBRACE },

		{ "$", TokenType::OP_REFERENCE }
	};

	std::unordered_map<std::string, TokenType> Lexer::Keywords = {
		{ "if", TokenType::KW_IF },
		{ "else", TokenType::KW_ELSE },
		{ "while", TokenType::KW_WHILE },
		{ "for", TokenType::KW_FOR },
		{ "fun", TokenType::KW_FUN },
		{ "let", TokenType::KW_LET },
		{ "return", TokenType::KW_RETURN },
		{ "break", TokenType::KW_BREAK },
		{ "continue", TokenType::KW_CONTINUE }
	};

	std::unordered_map<std::string, TokenType> Lexer::DataTypes = {
		{ "void", TokenType::TYPE_VOID },

		{ "bool", TokenType::TYPE_BOOL },
		{ "char", TokenType::TYPE_CHAR },

		{ "byte", TokenType::TYPE_BYTE },
		{ "int", TokenType::TYPE_INT },
		{ "long", TokenType::TYPE_LONG },

		{ "float", TokenType::TYPE_FLOAT },
		{ "double", TokenType::TYPE_DOUBLE }
	};

	std::string Lexer::GetOperatorLexeme(TokenType Type)
	{
		static std::unordered_map<TokenType, std::string> ReversedOperatorsMap;
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
			SkipComments();
			SkipSpaces();

			if (!IsValidPos())
				break;

			if (GetIdentifierToken(Tok))
				Tokens.Add(Tok);
			else if (GetNumberToken(Tok))
				Tokens.Add(Tok);
			else if (GetOperatorToken(Tok))
				Tokens.Add(Tok);
			else if (GetStringToken(Tok))
				Tokens.Add(Tok);
			else if (GetChar(Tok))
				Tokens.Add(Tok);
			else
			{
				SendError(LexErrorType::InvalidCharacter, Line, Column,
					{ std::string(1, CurrentChar()) });
				MovePos();
			}
		}

		//TokensArena.SetAutoReallocate(false);
	}

	void Lexer::WriteErrors(std::ostream &Os) const
	{
		for (const LexError& Error : Errors)
			Os << "LexError: " << Error.ToString() <<
				" At position: [" << Error.Line << ":" << Error.Column << "]\n";
	}

	void Lexer::WriteTokens(std::ostream &Os) const
	{
		for (const Token& Tok : Tokens)
			Os << Tok.ToString(Context) << std::endl;
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

	void Lexer::SkipComments()
	{
		while (true)
		{
			SkipSpaces();
			if (CurrentChar() == '/' && IsValidNextPos() && NextChar() == '/')
			{
				do
					MovePos();
				while (IsValidPos() && CurrentChar() != '\n');
			}
			else if (CurrentChar() == '/' && IsValidNextPos() && NextChar() == '*')
			{
				while (true)
				{
					if (IsValidPos() && CurrentChar() == '*' && IsValidNextPos() && NextChar() == '/')
					{
						MovePos(2);
						break;
					}

					if (!IsValidPos())
					{
						SendError(LexErrorType::UnterminatedBlockComment, Line, Column);
						break;
					}

					MovePos();
				}
			}
			else
				break;
		}
	}

	bool Lexer::GetIdentifierToken(Token &Tok)
	{
		size_t StartPos = Pos, StartLine = Line, StartCol = Column;

		if (UChar Ch = CurrentUChar(); std::isalpha(Ch) || Ch == '_')
			MovePos();
		else
			return false;

		while (IsValidPos())
		{
			if (UChar Ch = CurrentUChar(); std::isalpha(Ch) || Ch == '_' || std::isdigit(Ch))
			{
				MovePos();
				continue;
			}

			break;
		}

		if (StartPos == Pos)
			return false;

		StringRef LexemeRef(StartPos, Pos - StartPos);
		TokenType TokenType = TokenType::IDENTIFIER;

		llvm::StringRef Lexeme{ Code.c_str() + StartPos, Pos - StartPos };
		if (auto KwIter = Keywords.find(std::string(Lexeme)); KwIter != Keywords.end())
			TokenType = KwIter->second;
		else if (auto TypeIter = DataTypes.find(std::string(Lexeme)); TypeIter != DataTypes.end())
			TokenType = TypeIter->second;
		else if (Lexeme == "true")
			TokenType = TokenType::BOOL_TRUE;
		else if (Lexeme == "false")
			TokenType = TokenType::BOOL_FALSE;

		Tok = Token(
			TokenType, LexemeRef,
			StartPos, StartLine, StartCol);
		return true;
	}

	bool Lexer::GetNumberToken(Token &Tok)
	{

		if (CurrentChar() == '.')
		{
			if (IsValidNextPos())
			{
				UChar Ch = NextUChar();
				if (!isdigit(Ch))
				{
					Tok = Token(TokenType::OP_DOT, { Pos, 1 }, Pos, Line, Column);
					MovePos();
					return true;
				}
			}
			else
			{
				Tok = Token(TokenType::OP_DOT, { Pos, 1 }, Pos, Line, Column);
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
			UChar Ch = CurrentUChar();

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
						IsInvalidToken = true;
				}
				else
					HasDot = true;
			}
			else if (std::tolower(Ch) == 'e')
			{
				if (!HasDigit || HasExponent)
				{
					if (!IsInvalidToken)
						IsInvalidToken = true;
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
						IsInvalidToken = true;
				}
				else
					HasExponentSign = true;
			}
			else if (isalpha(Ch) || Ch == '_')
			{
				if (Suffix.Length == 0)
					Suffix.Ptr = Pos;

				Suffix.Length++;
			}
			else
				break;

			MovePos();
		}

		if (StartPos == Pos)
			return false;

		StringRef Lexeme( StartPos, Pos - StartPos);
		if (HasExponent && !HasExponentDigits)
		{
			if (!IsInvalidToken)
			{
				IsInvalidToken = true;
				SendError(LexErrorType::UnterminatedNumber, StartLine, StartCol);
			}
		}
		else if (IsInvalidToken)
		{
			SendError(LexErrorType::InvalidNumber, StartLine, StartCol,
					{ std::string(Code.c_str() + StartPos, Pos - StartPos) });
			Tok = Token(TokenType::INVALID, Lexeme,
						StartPos, StartLine, StartCol);

			return true;
		}

		TokenType TokenType;

		llvm::StringRef SuffixStr{ Code.c_str() + Suffix.Ptr, Suffix.Length };
		if (HasDot || HasExponent)
		{
			if (Suffix.Length == 0)
				TokenType = TokenType::DOUBLE_NUMBER;
			else if (SuffixStr == "f")
				TokenType = TokenType::FLOAT_NUMBER;
			else
			{
				SendError(LexErrorType::InvalidNumber, StartLine, StartCol,
					{ SuffixStr.str() });
				Tok = InvalidToken(StartPos, StartLine, StartCol);
				return true;
			}
		}
		else
		{
			if (Suffix.Length == 0)
				TokenType = TokenType::INT_NUMBER;
			else if (SuffixStr == "b")
				TokenType = TokenType::BYTE_NUMBER;
			else if (SuffixStr == "l")
				TokenType = TokenType::LONG_NUMBER;
			else
			{
				SendError(LexErrorType::InvalidNumber, StartLine, StartCol,
					{ std::string(Code.c_str() + Lexeme.Ptr, Lexeme.Length) });
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

		size_t Len = std::min(MaxOperatorSize, CodeSize - StartPos);
		StringRef OperatorLexeme( Pos, Len);

		while (Len > 0)
		{
			OperatorLexeme.Length = Len;

			if (auto Iter = Operators.find(std::string(
				Code.c_str() + OperatorLexeme.Ptr, Len)); Iter != Operators.end())
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

		bool InvalidEscape = false;
		if (Ch == '\\')
		{
			InvalidEscape = !GetEscape(CurrentChar(), Ch);
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

		Code.push_back(Ch);
		Tok = Token(InvalidEscape ? TokenType::INVALID : TokenType::CHAR,
			StringRef(Code.size() - 1, 1), StartPos, StartLine, StartCol);
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
				size_t Start = Code.size();
				Code.append(Str);
				Tok = Token(TokenType::STRING,
					{ Start, Str.size() }, StartPos, StartLine, StartCol);
				return true;
			}
			if (Ch == '\\')
			{
				MovePos();
				if (!IsValidPos())
				{
					size_t Start = Code.size();
					Code.append(Str);
					SendError(LexErrorType::UnterminatedEscape, StartLine, StartCol);
					Tok = Token(TokenType::INVALID,
						{ Start, Str.size() }, StartPos, StartLine, StartCol);
					return true;
				}

				if (char Escape; GetEscape(CurrentChar(), Escape))
					Str.push_back(Escape);
			}
			else if (Ch == '\n')
			{
				size_t Start = Code.size();
				Code.append(Str);
				SendError(LexErrorType::NewlineInString, StartLine, StartCol);
				Tok = InvalidToken({ Start, Str.size() }, StartPos, StartLine, StartCol);
				return true;
			}
			else
				Str.push_back(Ch);

			MovePos();
		}

		size_t Start = Code.size();
		Code.append(Str);
		SendError(LexErrorType::UnterminatedString, StartLine, StartCol);
		Tok = Token(TokenType::INVALID,
			{Start, Str.size()}, StartPos, StartLine, StartCol);
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
				SendError(LexErrorType::InvalidEscape, Line, Column, { std::string(1, Ch) });
				return false;
		}

		return true;
	}
}