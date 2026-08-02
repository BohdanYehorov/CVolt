//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_OPERATORTYPE_H
#define CVOLT_OPERATORTYPE_H

namespace Volt
{
	enum class OperatorType
	{
		Add,
		Sub,
		Mul,
		Div,
		Mod,
		Inc,
		Dec,
		Assign,
		AddAssign,
		SubAssign,
		MulAssign,
		DivAssign,
		ModAssign,
		AndAssign,
		OrAssign,
		XorAssign,
		LShiftAssign,
		RShiftAssign,
		Equal,
		NotEqual,
		Grater,
		GraterEqual,
		Less,
		LessEqual,
		LogicalAnd,
		LogicalOr,
		LogicalNot,
		BitAnd,
		BitOr,
		BitXor,
		BitNot,
		LShift,
		RShift,

		UnPlus,
		UnMinus,
		Unref,

		Unknown
	};
}

#endif //CVOLT_OPERATORTYPE_H