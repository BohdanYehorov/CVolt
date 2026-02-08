//
// Created by bohdan on 07.02.26.
//

#include "Volt/Core/Errors/ParseError.h"

namespace Volt
{
	std::string ParseError::ToString() const
	{
		using enum ParseErrorType;
		switch (Type)
		{
			case ExpectedToken:
				return std::format("Expected '{}', but got '{}'.", Context[0], Context[1]);
			case UnexpectedToken:
				return std::format("Unexpected '{}'.", Context[0]);
			case UnexpectedEOF:
				return "Unexpected end of file.";
			case ExpectedExpression:
				return "Expected expression.";
			case ExpectedInitializerExpression:
				return "Expected initializer expression after '='.";
			case ExpectedDeclaratorName:
				return "Expected declarator name.";
			case ExpectedFunctionBody:
				return "Expected function body.";
			case ExpectedStatement:
				return "Expected statement.";
			case ExpectedDataType:
				return "Expected data type.";
			case ReturnOutsideFunction:
				return "'return' cannot be used outside of a function.";
			case BreakOutsideLoop:
				return "'break' cannot be used outside of a loop.";
			case ContinueOutsideLoop:
				return "'continue' cannot be used outside of a loop.";
			case FunctionDefinitionNotAllowed:
				return "Function definition is not allowed here.";
			case ExpectedDeclaration:
				return "Expected a declaration.";
			default:
				break;
		}
		return "";
	}
}