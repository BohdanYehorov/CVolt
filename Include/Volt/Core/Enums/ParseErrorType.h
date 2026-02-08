//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_PARSEERRORTYPE_H
#define CVOLT_PARSEERRORTYPE_H

namespace Volt
{
	enum class ParseErrorType
	{
		UnexpectedToken,
		ExpectedToken,
		UnexpectedEOF,
		ExpectedExpression,
		ExpectedInitializerExpression,
		ExpectedDeclaratorName,
		ExpectedFunctionBody,
		ExpectedStatement,
		ExpectedDataType,
		ExpectedDeclaration,
		ReturnOutsideFunction,
		BreakOutsideLoop,
		ContinueOutsideLoop,
		FunctionDefinitionNotAllowed
	};
}

#endif //CVOLT_PARSEERRORTYPE_H