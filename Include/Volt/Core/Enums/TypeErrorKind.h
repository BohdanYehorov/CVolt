//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_TYPEERRORKIND_H
#define CVOLT_TYPEERRORKIND_H

namespace Volt
{
	enum class TypeErrorKind
	{
		UnknownType,
		InvalidType,
		TypeMissmatch,
		IncompatibleTypes,
		UndefinedVariable,
		UninitializedVariable,
		DoubleVariableDeclaration,
		Redeclaration,
		ImmutableAssignment,
		InvalidAssignment,
		InvalidBind,
		AssignNonLValue,
		NonLValue,
		AssignReadOnlyType,
		AssignmentTypeMismatch,
		InvalidBinaryOperator,
		BinaryOperandTypeMismatch,
		LogicalOperatorOnNonBool,
		ComparisonTypeMismatch,
		InvalidUnaryOperator,
		UnaryOperandTypeMismatch,
		ConditionNotBool,
		DuplicateFunction,
		NoFunctionOverload,
		InvalidReturnType,
		UndefinedFunction,
		ArgumentCountMismatch,
		ArgumentTypeMismatch,
		AmbiguousFunctionCall,
		InvalidCalleeType,
		CallingNonCallable,
		ReturnTypeMismatch,
		MissingReturn,
		VoidReturnValue,
		NonVoidMissingReturn,
		IndexingNonArray,
		IndexNotInteger,
		ArrayElementTypeMismatch,
		InvalidArrayLiteral,
		MemberNotIdentifier,
		AccessToNonClassType
	};
}

#endif //CVOLT_TYPEERRORKIND_H