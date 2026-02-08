//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_LEXERROR_H
#define CVOLT_LEXERROR_H

#include "Volt/Core/Enums/LexErrorType.h"
#include "Error.h"

namespace Volt
{
	struct LexError : Error
	{
		LexErrorType Type;
		LexError(LexErrorType Type, size_t Line, size_t Column, std::vector<std::string>&& Context)
			: Error(Line, Column, std::move(Context)), Type(Type) {}

		[[nodiscard]] std::string ToString() const override;
	};
}

#endif //CVOLT_LEXERROR_H