//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_TOKENTYPE_H
#define CVOLT_TOKENTYPE_H

namespace Volt
{
	enum class TokenType
	{
		Identifier,

		NumberI8,
		NumberI16,
		NumberI32,
		NumberI64,

		NumberU8,
		NumberU16,
		NumberU32,
		NumberU64,

		NumberF16,
		NumberF32,
		NumberF64,
		NumberF128,

		String,
		BoolTrue,
		BoolFalse,
		Char,
		NullPointer,

		Plus,
		Minus,
		Star,
		Slash,
		Percent,
		PlusPlus,
		MinusMinus,

		Equal,
		PlusEqual,
		MinusEqual,
		StarEqual,
		SlashEqual,
		PercentEqual,
		AmpEqual,
		PipeEqual,
		CaretEqual,
		LessLessEqual,
		GraterGraterEqual,

		EqualEqual,
		ExclaimEqual,
		Grater,
		GraterEqual,
		Less,
		LessEqual,

		AmpAmp,
		PipePipe,
		Exclaim,

		Amp,
		Pipe,
		Caret,
		Tilde,
		LessLess,
		GraterGrater,

		Dot,
		Question,
		Colon,
		Comma,
		Semicolon,

		LParen,
		RParen,
		LSquare,
		RSquare,
		LBrace,
		RBrace,
		Dollar,

		KwIf,
		KwElse,
		KwWhile,
		KwFor,
		KwFun,
		KwLet,
		KwReturn,
		KwBreak,
		KwContinue,
		KwClass,
		KwTo,
		KwForce,
		KwType,
		KwSizeOf,
		KwAlignOf,
		KwTypeOf,

		TypeConst,
		TypeVoid,

		TypeBool,
		TypeChar,

		TypeI8,
		TypeI16,
		TypeI32,
		TypeI64,

		TypeU8,
		TypeU16,
		TypeU32,
		TypeU64,

		TypeF16,
		TypeF32,
		TypeF64,
		TypeF128,

		Invalid,
		Unknown
	};
}

#endif //CVOLT_TOKENTYPE_H