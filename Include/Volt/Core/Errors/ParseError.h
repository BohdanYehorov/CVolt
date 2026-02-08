//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_PARSEERROR_H
#define CVOLT_PARSEERROR_H

#include "Volt/Core/Enums/ParseErrorType.h"
#include "Error.h"

namespace Volt
{
	struct ParseError : Error
	{
		ParseErrorType Type;
		ParseError(ParseErrorType Type, size_t Line, size_t Column, std::vector<std::string>&& Context)
			: Error(Line, Column, std::move(Context)), Type(Type) {}
		[[nodiscard]] std::string ToString() const override;
	};
}

#endif //CVOLT_PARSEERROR_H