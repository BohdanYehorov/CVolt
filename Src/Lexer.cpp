//
// Created by bohdan on 13.12.25.
//

#include "Volt/Core/Lexer/Lexer.h"

namespace Volt
{
	llvm::StringMap<TokenType> Lexer::Operators = {
		{ "+",   TokenType::Plus },
		{ "-",   TokenType::Minus },
		{ "*",   TokenType::Star },
		{ "/",   TokenType::Slash },
		{ "%",   TokenType::Percent },
		{ "++",  TokenType::PlusPlus },
		{ "--",  TokenType::MinusMinus },

		{ "=",   TokenType::Equal },
		{ "+=",  TokenType::PlusEqual },
		{ "-=",  TokenType::MinusEqual },
		{ "*=",  TokenType::StarEqual },
		{ "/=",  TokenType::SlashEqual },
		{ "%=",  TokenType::PercentEqual },
		{ "&=",  TokenType::AmpEqual },
		{ "|=",  TokenType::PipeEqual },
		{ "^=",  TokenType::CaretEqual },
		{ "<<=", TokenType::LessLessEqual },
		{ ">>=", TokenType::GraterGraterEqual },

		{ "==",  TokenType::EqualEqual },
		{ "!=",  TokenType::ExclaimEqual },
		{ ">",   TokenType::Grater },
		{ ">=",  TokenType::GraterEqual },
		{ "<",   TokenType::Less } ,
		{ "<=",  TokenType::LessEqual },

		{ "&&",  TokenType::AmpAmp },
		{ "||",  TokenType::PipePipe },
		{ "!",   TokenType::Exclaim },

		{ "&",   TokenType::Amp },
		{ "|",   TokenType::Pipe },
		{ "^",   TokenType::Caret },
		{ "~",   TokenType::Tilde },
		{ "<<",  TokenType::LessLess },
		{ ">>",  TokenType::GraterGrater },

		{ ".",   TokenType::Dot },
		{ "?",   TokenType::Question },
		{ ":",   TokenType::Colon },
		{ ",",   TokenType::Comma },
		{ ";",   TokenType::Semicolon },

		{ "(",   TokenType::LParen },
		{ ")",   TokenType::RParen },
		{"[",    TokenType::LSquare },
		{ "]",   TokenType::RSquare },
		{ "{",   TokenType::LBrace },
		{ "}",   TokenType::RBrace },

		{ "$",   TokenType::Dollar }
	};

	llvm::StringMap<TokenType> Lexer::Keywords {
		{ "if",       TokenType::KwIf },
		{ "else",     TokenType::KwElse },
		{ "while",    TokenType::KwWhile },
		{ "for",      TokenType::KwFor },
		{ "fun",      TokenType::KwFun },
		{ "let",      TokenType::KwLet },
		{ "return",   TokenType::KwReturn },
		{ "break",    TokenType::KwBreak },
		{ "continue", TokenType::KwContinue },
		{ "class",    TokenType::KwClass },
		{ "to", TokenType::KwTo }
	};

	llvm::StringMap<TokenType> Lexer::DataTypes = {
		{"const", TokenType::TypeConst },
		{ "void", TokenType::TypeVoid },

		{ "bool", TokenType::TypeBool },
		{ "char", TokenType::TypeChar },

		{ "i8",  TokenType::TypeI8 },
		{ "i16", TokenType::TypeI16 },
		{ "i32", TokenType::TypeI32 },
		{ "i64", TokenType::TypeI64 },

		{ "u8",  TokenType::TypeU8 },
		{ "u16", TokenType::TypeU16 },
		{ "u32", TokenType::TypeU32 },
		{ "u64", TokenType::TypeU64 },

		{ "f16",  TokenType::TypeF16 },
		{ "f32",  TokenType::TypeF32 },
		{ "f64",  TokenType::TypeF64 },
		{ "f128", TokenType::TypeF128 }
	};

	llvm::StringMap<TokenType> Lexer::IntNumberLiterals = {
		{ "i8",  TokenType::NumberI8 },
		{ "i16", TokenType::NumberI16 },
		{ "i32", TokenType::NumberI32 },
		{ "i64", TokenType::NumberI64 },

		{ "u8", TokenType::NumberU8 },
		{ "u16", TokenType::NumberU16 },
		{ "u32", TokenType::NumberU32 },
		{ "u64", TokenType::NumberU64 }
	};

	llvm::StringMap<TokenType> Lexer::FloatNumberLiterals = {
		{ "f16",  TokenType::NumberF16 },
		{ "f32",  TokenType::NumberF32 },
		{ "f64",  TokenType::NumberF64 },
		{ "f128", TokenType::NumberF128 }
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
		TokenType TokenType = TokenType::Identifier;

		llvm::StringRef Lexeme{ Code.CStr() + StartPos, Pos - StartPos };
		if (auto KwIter = Keywords.find(Lexeme); KwIter != Keywords.end())
			TokenType = KwIter->second;
		else if (auto TypeIter = DataTypes.find(Lexeme); TypeIter != DataTypes.end())
			TokenType = TypeIter->second;
		else if (Lexeme == "true")
			TokenType = TokenType::BoolTrue;
		else if (Lexeme == "false")
			TokenType = TokenType::BoolFalse;
		else if (Lexeme == "null")
			TokenType = TokenType::NullPointer;

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
					Tok = Token(TokenType::Dot, { Pos, 1 }, Pos, Line, Column);
					MovePos();
					return true;
				}
			}
			else
			{
				Tok = Token(TokenType::Dot, { Pos, 1 }, Pos, Line, Column);
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
			Tok = Token(TokenType::Invalid, Lexeme,
						StartPos, StartLine, StartCol);

			return true;
		}

		TokenType TokenType;

		llvm::StringRef SuffixLit;
		if (!GetNumberSuffixLiteral(SuffixLit))
			TokenType = (HasDot || HasExponent ? TokenType::NumberF64 : TokenType::NumberI32);
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
						{ std::string(Code.CStr() + Lexeme.Index, Lexeme.Length) });
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
				Code.CStr() + OperatorLexeme.Index, Len)); Iter != Operators.end())
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
		Tok = Token(InvalidEscape ? TokenType::Invalid : TokenType::Char,
			StringRef(Code.Length() - 1, 1), StartPos, StartLine, StartCol);
		return true;
	}

	bool Lexer::GetStringToken(Token &Tok)
	{
		if (CurrentChar() != '"')
			return false;

		size_t StartPos = Pos, StartLine = Line, StartCol = Column;
		MovePos();

		size_t StringStart = Code.Length();

		while (IsValidPos())
		{
			char Ch = CurrentChar();
			switch (Ch)
			{
				case '"':
				{
					MovePos();
					Tok = Token(TokenType::String,{ StringStart,
						Code.Length() - StringStart }, StartPos, StartLine, StartCol);
					return true;
				}
				case '\\':
				{
					MovePos();
					if (!IsValidPos())
					{
						SendError(LexErrorType::UnterminatedEscape, StartLine, StartCol);
						Tok = InvalidToken({ StringStart,
							Code.Length() - StringStart }, StartPos, StartLine, StartCol);
						return true;
					}

					if (char Escape; GetEscape(CurrentChar(), Escape))
						Code.Add(Escape);
					break;
				}
				case '\n':
				{
					SendError(LexErrorType::NewlineInString, StartLine, StartCol);
					Tok = InvalidToken({ StringStart,
						Code.Length() - StringStart }, StartPos, StartLine, StartCol);
					return true;
				}
				default:
					Code.Add(Ch);
					break;
			}

			MovePos();
		}

		SendError(LexErrorType::UnterminatedString, StartLine, StartCol);
		Tok = InvalidToken({ StringStart,
			Code.Length() - StringStart }, StartPos, StartLine, StartCol);
		return true;
	}

	bool Lexer::GetEscape(char Ch, char& Escape)
	{
		switch (Ch)
		{
			case '\'': Escape = '\''; break;
			case '"':  Escape = '"';  break;
			case '\\': Escape = '\\'; break;
			case '?':  Escape = '\?'; break;
			case 'a':  Escape = '\a'; break;
			case 'b':  Escape = '\b'; break;
			case 'f':  Escape = '\f'; break;
			case 'n':  Escape = '\n'; break;
			case 'r':  Escape = '\r'; break;
			case 't':  Escape = '\t'; break;
			case 'v':  Escape = '\v'; break;
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