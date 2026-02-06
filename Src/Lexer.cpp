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
		{ "}", Token::OP_RBRACE },

		{ "$", Token::OP_REFERENCE }
	};

	std::unordered_map<std::string, Token::TokenType> Lexer::Keywords = {
		{ "if", Token::KW_IF },
		{ "else", Token::KW_ELSE },
		{ "while", Token::KW_WHILE },
		{ "for", Token::KW_FOR },
		{ "fun", Token::KW_FUN },
		{ "let", Token::KW_LET },
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

	// Lexer::Lexer(const std::string &Expr) : Code(Expr)
	// {
	// 	TokensArena.SetAutoReallocate(true);
	// 	ExprRef = TokensArena.Write(Expr);
	// 	StringStoragePtr = TokensArena.GetWritePtr();
	// }

	Lexer::Lexer(CompilationContext &Context)
		: Context(Context), Code(Context.Code), CodeSize(Code.size()), Tokens(Context.Tokens)
	{
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
		Token::TokenType TokenType = Token::IDENTIFIER;

		llvm::StringRef Lexeme{ Code.c_str() + StartPos, Pos - StartPos };
		if (auto KwIter = Keywords.find(std::string(Lexeme)); KwIter != Keywords.end())
			TokenType = KwIter->second;
		else if (auto TypeIter = DataTypes.find(std::string(Lexeme)); TypeIter != DataTypes.end())
			TokenType = TypeIter->second;
		else if (Lexeme == "true")
			TokenType = Token::BOOL_TRUE;
		else if (Lexeme == "false")
			TokenType = Token::BOOL_FALSE;

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
			Tok = Token(Token::INVALID, Lexeme,
						StartPos, StartLine, StartCol);

			return true;
		}

		Token::TokenType TokenType;

		llvm::StringRef SuffixStr{ Code.c_str() + Suffix.Ptr, Suffix.Length };
		if (HasDot || HasExponent)
		{
			if (Suffix.Length == 0)
				TokenType = Token::DOUBLE_NUMBER;
			else if (SuffixStr == "f")
				TokenType = Token::FLOAT_NUMBER;
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
				TokenType = Token::INT_NUMBER;
			else if (SuffixStr == "b")
				TokenType = Token::BYTE_NUMBER;
			else if (SuffixStr == "l")
				TokenType = Token::LONG_NUMBER;
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
		Tok = Token(InvalidEscape ? Token::INVALID : Token::CHAR,
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
				Tok = Token(Token::STRING,
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
					Tok = Token(Token::INVALID,
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
		Tok = Token(Token::INVALID,
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