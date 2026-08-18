//
// Created by bohdan on 06.02.26.
//

#include "Volt/Core/Lexer/Token.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include <sstream>

#define GEN_CASE_TO_STRING(Op) case TokenType::Op: TypeStr = #Op; break;

namespace Volt
{
	std::string Token::ToString(const CompilationContext &Context) const
	{
		std::string TypeStr;

		switch (Type)
		{
			GEN_CASE_TO_STRING(Identifier)
			GEN_CASE_TO_STRING(NumberI8)
			GEN_CASE_TO_STRING(NumberI16)
			GEN_CASE_TO_STRING(NumberI32)
			GEN_CASE_TO_STRING(NumberI64)
			GEN_CASE_TO_STRING(NumberU8)
			GEN_CASE_TO_STRING(NumberU16)
			GEN_CASE_TO_STRING(NumberU32)
			GEN_CASE_TO_STRING(NumberU64)
			GEN_CASE_TO_STRING(NumberF16)
			GEN_CASE_TO_STRING(NumberF32)
			GEN_CASE_TO_STRING(NumberF64)
			GEN_CASE_TO_STRING(NumberF128)
			GEN_CASE_TO_STRING(String)
			GEN_CASE_TO_STRING(BoolTrue)
			GEN_CASE_TO_STRING(BoolFalse)
			GEN_CASE_TO_STRING(Char)
			GEN_CASE_TO_STRING(NullPointer)
			GEN_CASE_TO_STRING(Plus)
			GEN_CASE_TO_STRING(Minus)
			GEN_CASE_TO_STRING(Star)
			GEN_CASE_TO_STRING(Slash)
			GEN_CASE_TO_STRING(Percent)
			GEN_CASE_TO_STRING(PlusPlus)
			GEN_CASE_TO_STRING(MinusMinus)
			GEN_CASE_TO_STRING(Equal)
			GEN_CASE_TO_STRING(PlusEqual)
			GEN_CASE_TO_STRING(MinusEqual)
			GEN_CASE_TO_STRING(StarEqual)
			GEN_CASE_TO_STRING(SlashEqual)
			GEN_CASE_TO_STRING(PercentEqual)
			GEN_CASE_TO_STRING(AmpEqual)
			GEN_CASE_TO_STRING(PipeEqual)
			GEN_CASE_TO_STRING(CaretEqual)
			GEN_CASE_TO_STRING(LessLessEqual)
			GEN_CASE_TO_STRING(GraterGraterEqual)
			GEN_CASE_TO_STRING(EqualEqual)
			GEN_CASE_TO_STRING(ExclaimEqual)
			GEN_CASE_TO_STRING(Grater)
			GEN_CASE_TO_STRING(GraterEqual)
			GEN_CASE_TO_STRING(Less)
			GEN_CASE_TO_STRING(LessEqual)
			GEN_CASE_TO_STRING(AmpAmp)
			GEN_CASE_TO_STRING(PipePipe)
			GEN_CASE_TO_STRING(Exclaim)
			GEN_CASE_TO_STRING(Amp)
			GEN_CASE_TO_STRING(Pipe)
			GEN_CASE_TO_STRING(Caret)
			GEN_CASE_TO_STRING(Tilde)
			GEN_CASE_TO_STRING(LessLess)
			GEN_CASE_TO_STRING(GraterGrater)
			GEN_CASE_TO_STRING(Dot)
			GEN_CASE_TO_STRING(Question)
			GEN_CASE_TO_STRING(Colon)
			GEN_CASE_TO_STRING(Comma)
			GEN_CASE_TO_STRING(Semicolon)
			GEN_CASE_TO_STRING(LParen)
			GEN_CASE_TO_STRING(RParen)
			GEN_CASE_TO_STRING(LSquare)
			GEN_CASE_TO_STRING(RSquare)
			GEN_CASE_TO_STRING(LBrace)
			GEN_CASE_TO_STRING(RBrace)
			GEN_CASE_TO_STRING(Dollar)
			GEN_CASE_TO_STRING(KwIf)
			GEN_CASE_TO_STRING(KwElse)
			GEN_CASE_TO_STRING(KwWhile)
			GEN_CASE_TO_STRING(KwFor)
			GEN_CASE_TO_STRING(KwFun)
			GEN_CASE_TO_STRING(KwLet)
			GEN_CASE_TO_STRING(KwReturn)
			GEN_CASE_TO_STRING(KwBreak)
			GEN_CASE_TO_STRING(KwContinue)
			GEN_CASE_TO_STRING(KwClass)
			GEN_CASE_TO_STRING(KwTo)
			GEN_CASE_TO_STRING(KwForce)
			GEN_CASE_TO_STRING(KwType)
			GEN_CASE_TO_STRING(KwSizeOf)
			GEN_CASE_TO_STRING(KwAlignOf)
			GEN_CASE_TO_STRING(KwTypeOf)
			GEN_CASE_TO_STRING(KwImpl)
			GEN_CASE_TO_STRING(TypeConst)
			GEN_CASE_TO_STRING(TypeVoid)
			GEN_CASE_TO_STRING(TypeBool)
			GEN_CASE_TO_STRING(TypeChar)
			GEN_CASE_TO_STRING(TypeI8)
			GEN_CASE_TO_STRING(TypeI16)
			GEN_CASE_TO_STRING(TypeI32)
			GEN_CASE_TO_STRING(TypeI64)
			GEN_CASE_TO_STRING(TypeU8)
			GEN_CASE_TO_STRING(TypeU16)
			GEN_CASE_TO_STRING(TypeU32)
			GEN_CASE_TO_STRING(TypeU64)
			GEN_CASE_TO_STRING(TypeF16)
			GEN_CASE_TO_STRING(TypeF32)
			GEN_CASE_TO_STRING(TypeF64)
			GEN_CASE_TO_STRING(TypeF128)
			GEN_CASE_TO_STRING(Invalid)
			GEN_CASE_TO_STRING(Unknown)
		}

		std::stringstream SStr;

		SStr << "Token Kind: " << TypeStr << ", Lexeme: '" << Context.GetTokenLexeme(Lexeme).str() <<
			"', Pos: " << Pos << ", Line: " << Line << ", Column: " << Column;
		return SStr.str();
	}

#undef GEN_CASE_TO_STRING
}