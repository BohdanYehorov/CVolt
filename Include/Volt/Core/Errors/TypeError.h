//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_TYPEERROR_H
#define CVOLT_TYPEERROR_H

#include "Volt/Core/Enums/TypeErrorKind.h"
#include "Error.h"

namespace Volt
{
	struct TypeError : Error
	{
		TypeErrorKind Kind;
		TypeError(TypeErrorKind Kind, size_t Line, size_t Column, std::vector<std::string>&& Context)
			: Error(Line, Column, std::move(Context)), Kind(Kind) {}
		[[nodiscard]] std::string ToString() const override;
	};
}

#endif //CVOLT_TYPEERROR_H