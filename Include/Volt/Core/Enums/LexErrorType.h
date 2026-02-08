//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_LEXERRORTYPE_H
#define CVOLT_LEXERRORTYPE_H

namespace Volt
{
	enum class LexErrorType
	{
		InvalidCharacter,
		InvalidNumber,
		UnterminatedNumber,

		UnterminatedString,
		InvalidEscape,
		UnterminatedEscape,
		NewlineInString,

		InvalidCharacterLiteral,
		UnterminatedCharacterLiteral,

		UnterminatedBlockComment,
		NestedBlockComment,

		InvalidIdentifier,
		KeywordAsIdentifier,

		UnknownOperator,
		IncompleteOperator,

		InvalidDelimiter,
		UnexpectedEOF,

		InvalidUnicode,
		UnexpectedBOM,

		InternalError,
		TokenCreationFailed,
		InfiniteLoop
	};
}

#endif //CVOLT_LEXERRORTYPE_H