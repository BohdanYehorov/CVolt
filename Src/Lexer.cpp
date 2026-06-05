//
// Created by bohdan on 13.12.25.
//

#include "Volt/Core/Lexer/Lexer.h"

#include <complex>
#include <sstream>

namespace Volt
{
	llvm::StringMap<TokenType> Lexer::Operators = {
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

	llvm::StringMap<TokenType> Lexer::Keywords {
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

	llvm::StringMap<TokenType> Lexer::DataTypes = {
		{"const", TokenType::TYPE_CONST },
		{ "void", TokenType::TYPE_VOID },

		{ "bool", TokenType::TYPE_BOOL },
		{ "char", TokenType::TYPE_CHAR },

		{ "i8", TokenType::TYPE_I8 },
		{ "i16", TokenType::TYPE_I16 },
		{ "i32", TokenType::TYPE_I32 },
		{ "i64", TokenType::TYPE_I64 },

		{ "u8", TokenType::TYPE_U8 },
		{ "u16", TokenType::TYPE_U16 },
		{ "u32", TokenType::TYPE_U32 },
		{ "u64", TokenType::TYPE_U64 },

		{ "f16", TokenType::TYPE_F16 },
		{ "f32", TokenType::TYPE_F32 },
		{ "f64", TokenType::TYPE_F64 },
		{ "f128", TokenType::TYPE_F128 }
	};

	llvm::StringMap<TokenType> Lexer::IntNumberLiterals = {
		{ "i8", TokenType::I8_NUMBER },
		{ "i16", TokenType::I16_NUMBER },
		{ "i32", TokenType::I32_NUMBER },
		{ "i64", TokenType::I64_NUMBER }
	};

	llvm::StringMap<TokenType> Lexer::FloatNumberLiterals = {
		{ "f16", TokenType::F16_NUMBER },
		{ "f32", TokenType::F32_NUMBER },
		{ "f64", TokenType::F64_NUMBER },
		{ "f128", TokenType::F128_NUMBER }
	};

	std::string Lexer::GetOperatorLexeme(TokenType Type)
	{
		static llvm::DenseMap<TokenType, std::string> ReversedOperatorsMap;
		if (ReversedOperatorsMap.empty())
		{
			for (const auto& [Lexeme, TokenType] : Operators)
				ReversedOperatorsMap[TokenType] = Lexeme;
		}

		if (auto Iter = ReversedOperatorsMap.find(Type); Iter != ReversedOperatorsMap.end())
			return Iter->second;
		return "";
	}

	bool Lexer::IsOperatorChar(UChar Ch)
	{
		static constexpr UInt8 OperatorChars[] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		};

		return OperatorChars[Ch];
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
			else if (GetOperatorToken(Tok))
				Tokens.Add(Tok);
			else if (GetNumberToken(Tok))
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

		llvm::StringRef Lexeme{ Code.CStr() + StartPos, Pos - StartPos };
		if (auto KwIter = Keywords.find(Lexeme); KwIter != Keywords.end())
			TokenType = KwIter->second;
		else if (auto TypeIter = DataTypes.find(Lexeme); TypeIter != DataTypes.end())
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
					{ std::string(Code.CStr() + StartPos, Pos - StartPos) });
			Tok = Token(TokenType::INVALID, Lexeme,
						StartPos, StartLine, StartCol);

			return true;
		}

		TokenType TokenType;

		llvm::StringRef SuffixLit;
		if (!GetNumberSuffixLiteral(SuffixLit))
			TokenType = (HasDot || HasExponent ? TokenType::F64_NUMBER : TokenType::I32_NUMBER);
		else
		{
			if (HasDot || HasExponent)
			{
				if (auto Iter = FloatNumberLiterals.find(SuffixLit); Iter != FloatNumberLiterals.end())
					TokenType = Iter->second;
				else
				{
					SendError(LexErrorType::InvalidNumber, StartLine, StartCol,
						{ SuffixLit.str() });
					Tok = InvalidToken(StartPos, StartLine, StartCol);
					return true;
				}
			}
			else
			{
				if (auto Iter = IntNumberLiterals.find(SuffixLit); Iter != IntNumberLiterals.end())
					TokenType = Iter->second;
				else
				{
					SendError(LexErrorType::InvalidNumber, StartLine, StartCol,
						{ std::string(Code.CStr() + Lexeme.Ptr, Lexeme.Length) });
					Tok = InvalidToken(StartPos, StartLine, StartCol);
					return true;
				}
			}
		}

		Tok = Token(TokenType, Lexeme,
						StartPos, StartLine, StartCol);

		return true;
	}

	bool Lexer::GetOperatorToken(Token &Tok)
	{
		if (!IsOperatorChar(CurrentChar()))
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

			if (auto Iter = Operators.find(llvm::StringRef(
				Code.CStr() + OperatorLexeme.Ptr, Len)); Iter != Operators.end())
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

		Code.Add(Ch);
		Tok = Token(InvalidEscape ? TokenType::INVALID : TokenType::CHAR,
			StringRef(Code.Length() - 1, 1), StartPos, StartLine, StartCol);
		return true;
	}

	bool Lexer::GetStringToken(Token &Tok)
	{
		if (CurrentChar() != '"')
			return false;

		size_t StartPos = Pos, StartLine = Line, StartCol = Column;
		MovePos();

		String Str;

		while (IsValidPos())
		{
			char Ch = CurrentChar();
			if (Ch == '"')
			{
				MovePos();
				size_t Start = Code.Length();
				Code.Append(Str);
				Tok = Token(TokenType::STRING,
					{ Start, Str.Length() }, StartPos, StartLine, StartCol);
				return true;
			}
			if (Ch == '\\')
			{
				MovePos();
				if (!IsValidPos())
				{
					size_t Start = Code.Length();
					Code.Append(Str);
					SendError(LexErrorType::UnterminatedEscape, StartLine, StartCol);
					Tok = Token(TokenType::INVALID,
						{ Start, Str.Length() }, StartPos, StartLine, StartCol);
					return true;
				}

				if (char Escape; GetEscape(CurrentChar(), Escape))
					Str.Add(Escape);
			}
			else if (Ch == '\n')
			{
				size_t Start = Code.Length();
				Code.Append(Str);
				SendError(LexErrorType::NewlineInString, StartLine, StartCol);
				Tok = InvalidToken({ Start, Str.Length() }, StartPos, StartLine, StartCol);
				return true;
			}
			else
				Str.Add(Ch);

			MovePos();
		}

		size_t Start = Code.Length();
		Code.Append(Str);
		SendError(LexErrorType::UnterminatedString, StartLine, StartCol);
		Tok = Token(TokenType::INVALID,
			{Start, Str.Length()}, StartPos, StartLine, StartCol);
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

	bool Lexer::GetNumberSuffixLiteral(llvm::StringRef &Lit)
	{
		if (!std::isalpha(CurrentUChar()) && CurrentChar() != '_')
			return false;

		size_t StartPos = Pos;

		while (IsValidPos())
		{
			if (UChar Ch = CurrentUChar(); std::isalpha(Ch) || Ch == '_' || std::isdigit(Ch))
			{
				MovePos();
				continue;
			}

			break;
		}

		Lit = llvm::StringRef(Code.CStr() + StartPos, Pos - StartPos);
		return true;
	}
}
