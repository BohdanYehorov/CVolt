//
// Created by bohdan on 07.02.26.
//

#include "Volt/Core/Errors/TypeError.h"

namespace Volt
{
	std::string TypeError::ToString() const
	{
		using enum TypeErrorKind;
		switch (Kind)
		{
			case UnknownType:
				return std::format("Type '{}' is not defined.", Context[0]);
			case InvalidType:
				return std::format("Type '{}' is invalid.", Context[0]);
			case TypeMissmatch:
				return std::format("Expected '{}', got '{}'.", Context[0], Context[1]);
			case IncompatibleTypes:
				return std::format("Cannot convert '{}' to '{}'.", Context[0], Context[1]);
			case UndefinedVariable:
				return std::format("Variable '{}' is undefined.", Context[0]);
			case UninitializedVariable:
				return std::format("Variable '{}' is uninitialized.", Context[0]);
			case Redeclaration:
				return std::format("Redeclaration '{}.'", Context[0]);
			case ImmutableAssignment:
				return std::format("Cannot assign '{}.'", Context[0]);
			case InvalidAssignment:
				return std::format("Cannot assign values of this type: '{}'.", Context[0]);
			case AssignmentTypeMismatch:
				return std::format("Cannot initialize local variable '{}' of type {} with {}.",
					Context[0], Context[1], Context[2]);
			case InvalidBinaryOperator:
				return std::format("Operator '{}' not defined for {}.", Context[0], Context[1]);
			case UndefinedFunction:
				return std::format("Function '{}' is undefined.", Context[0]);
			default:
				return "";
		}

	}
}