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
				return std::format("Type '{}' is not defined.", Context.at(0));
			case InvalidType:
				return std::format("Type '{}' is invalid.", Context.at(0));
			case TypeMissmatch:
				return std::format("Expected '{}', got '{}'.", Context.at(0), Context.at(1));
			case IncompatibleTypes:
				return std::format("Cannot convert '{}' to '{}'.", Context.at(0), Context.at(1));
			case UndefinedVariable:
				return std::format("Variable '{}' is undefined.", Context.at(0));
			case UninitializedVariable:
				return std::format("Variable '{}' is uninitialized.", Context.at(0));
			case Redeclaration:
				return std::format("Redeclaration '{}.'", Context.at(0));
			case ImmutableAssignment:
				return std::format("Cannot assign '{}.'", Context.at(0));
			case InvalidAssignment:
				return std::format("Cannot assign values of this type: '{}'.", Context.at(0));
			case AssignmentTypeMismatch:
				return std::format("Cannot initialize local variable '{}' of type {} with {}.",
					Context.at(0), Context.at(1), Context.at(2));
			case InvalidBinaryOperator:
				return std::format("Operator '{}' not defined for {}.", Context.at(0), Context.at(1));
			case UndefinedFunction:
				return std::format("Function '{}' is undefined.", Context.at(0));
			default:
				return "";
		}

	}
}