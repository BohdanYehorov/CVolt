//
// Created by bohdan on 28.01.26.
//

#include "Volt/Core/TypeChecker/ExprResult.h"

namespace Volt
{
	ExprResult *ExprResult::CreateRaw(CompilationContext &CContext)
	{
		return CContext.MainArena.Create<ExprResult>();
	}

	ExprResult *ExprResult::CreateInteger(QualType IntType, Int64 Integer, Arena& MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = IntType;
		Value->Int = Integer;
		Value->IsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateFloat(QualType FloatType, double Float, Arena& MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = FloatType;
		Value->Float = Float;
		Value->IsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateBool(QualType BoolType, bool Bool, Arena& MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = BoolType;
		Value->Bool = Bool;
		Value->IsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateChar(QualType CharType, char Char, Arena &MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = CharType;
		Value->Char = Char;
		Value->IsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateEmpty(QualType Type, Arena& MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = Type;
		Value->IsEmpty = true;
		return Value;
	}

	ExprResult *ExprResult::CastTo(QualType To, bool Explicit, CompilationContext &CContext)
	{
		if (Type == To)
			return this;

		if (!Type.CastTo(To, Explicit))
			return nullptr;

		if (IsEmpty)
			return CreateEmpty(To, CContext.MainArena);

		switch (Type->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return CastBooleanTo(To, Explicit, CContext);

			case TypeCategory::CHAR:
				return CastCharTo(To, Explicit, CContext);

			case TypeCategory::INTEGER:
				return CastIntegerTo(To, Explicit, CContext);

			case TypeCategory::FLOATING_POINT:
				return CastFloatTo(To, Explicit, CContext);

			default:
				return nullptr;
		}
	}

#define RESOLVE_OP_FOR_ALL_TYPES(Op) switch (LeftType->GetCategory()) \
	{ \
		case TypeCategory::INTEGER: \
			return CreateFromType(ResultType, Left->Int Op Right->Int, CContext.MainArena); \
		case TypeCategory::FLOATING_POINT: \
			return CreateFromType(ResultType, Left->Float Op Right->Float, CContext.MainArena); \
		case TypeCategory::BOOLEAN: \
			return CreateFromType(ResultType, Left->Bool Op Right->Bool, CContext.MainArena); \
		case TypeCategory::CHAR: \
			return CreateFromType(ResultType, Left->Char Op Right->Char, CContext.MainArena); \
		default: \
			return nullptr; \
	}

#define RESOLVE_OP_FOR_INT_FLOAT_TYPES(Op) switch (LeftType->GetCategory()) \
	{ \
		case TypeCategory::INTEGER: \
			return CreateFromType(ResultType, Left->Int Op Right->Int, CContext.MainArena); \
		case TypeCategory::FLOATING_POINT: \
			return CreateFromType(ResultType, Left->Float Op Right->Float, CContext.MainArena); \
		case TypeCategory::CHAR: \
			return CreateFromType(ResultType, Left->Char Op Right->Char, CContext.MainArena); \
		default: \
			return nullptr; \
	}

#define RESOLVE_OP_FOR_INT_TYPES(Op) {switch (LeftType->GetCategory()) \
	{ \
		case TypeCategory::INTEGER: \
			return CreateFromType(ResultType, Left->Int Op Right->Int, CContext.MainArena); \
		case TypeCategory::CHAR: \
			return CreateFromType(ResultType, Left->Char Op Right->Char, CContext.MainArena); \
		default: \
			return nullptr; \
	}}

	ExprResult *ExprResult::ResolveBinary(ExprResult *&Left, ExprResult *&Right, OperatorType Op,
	                                      CompilationContext& CContext)
	{
		using enum OperatorType;

		QualType LeftType = Left->Type;
		QualType RightType = Right->Type;

		QualType ResultType = Operator::ResolveBinary(LeftType, RightType, Op, CContext);

		// if (!Left->ImplicitCast(LeftType) || !Right->ImplicitCast(RightType))
		// 	return nullptr;

		Left = Left->ImplicitCast(LeftType, CContext);
		Right = Right->ImplicitCast(RightType, CContext);

		if (!Left || !Right)
			return nullptr;

		if (Left->IsEmpty || Right->IsEmpty)
			return CreateEmpty(ResultType, CContext.MainArena);

		switch (Op)
		{
			case ADD: RESOLVE_OP_FOR_INT_FLOAT_TYPES(+)
			case SUB: RESOLVE_OP_FOR_INT_FLOAT_TYPES(-)
			case MUL: RESOLVE_OP_FOR_INT_FLOAT_TYPES(*)
			case DIV: RESOLVE_OP_FOR_INT_FLOAT_TYPES(/)
			case MOD: RESOLVE_OP_FOR_INT_TYPES(%)
			case BIT_AND: RESOLVE_OP_FOR_INT_TYPES(&)
			case BIT_OR: RESOLVE_OP_FOR_INT_TYPES(|)
			case BIT_XOR: RESOLVE_OP_FOR_INT_TYPES(^)
			case RSHIFT: RESOLVE_OP_FOR_INT_TYPES(>>)
			case LSHIFT: RESOLVE_OP_FOR_INT_TYPES(<<)

			case EQ: RESOLVE_OP_FOR_ALL_TYPES(==)
			case NEQ: RESOLVE_OP_FOR_ALL_TYPES(!=)

			case GT: RESOLVE_OP_FOR_INT_FLOAT_TYPES(>)
			case GTE: RESOLVE_OP_FOR_INT_FLOAT_TYPES(>=)
			case LT: RESOLVE_OP_FOR_INT_FLOAT_TYPES(<)
			case LTE: RESOLVE_OP_FOR_INT_FLOAT_TYPES(<=)

			default: return nullptr;
		}
	}

	ExprResult *ExprResult::ResolveUnary(ExprResult *&Operand, OperatorType Op, CompilationContext &CContext)
	{
		using enum TypeCategory;

		if (!Operand)
			return nullptr;

		if (Operand->IsEmpty)
		{
			QualType OperandType = Operand->Type;
			if (auto Type = Operator::ResolveUnary(OperandType, Op))
				return CreateEmpty(Type, CContext.MainArena);

			return nullptr;
		}

		TypeCategory OperandTypeCategory = Operand->Type->GetCategory();
		switch (Op)
		{
			case OperatorType::ADD:
			{
				switch (OperandTypeCategory)
				{
					case CHAR:
					case INTEGER:
					case FLOATING_POINT:
						return CContext.MainArena.Create<ExprResult>(*Operand);
					default:
						return nullptr;
				}
			}
			case OperatorType::SUB:
			{
				switch (OperandTypeCategory)
				{
					case CHAR:
						return CreateChar(Operand->Type, -Operand->Char, CContext.MainArena);
					case INTEGER:
						return CreateInteger(Operand->Type, -Operand->Int, CContext.MainArena);
					case FLOATING_POINT:
						return CreateFloat(Operand->Type, -Operand->Float, CContext.MainArena);
					default:
						return nullptr;
				}
			}
			case OperatorType::BIT_NOT:
			{
				switch (OperandTypeCategory)
				{
					case CHAR:
						return CreateChar(Operand->Type, ~Operand->Char, CContext.MainArena);
					case INTEGER:
						return CreateInteger(Operand->Type, ~Operand->Int, CContext.MainArena);
					default:
						return nullptr;
				}
			}
			case OperatorType::LOGICAL_NOT:
			{
				if (auto CastedOp = Operand->ImplicitCast(
					QualType(CContext.GetBoolType(), QualType::CONST), CContext))
					return CreateBool(CastedOp->Type, !CastedOp->Bool, CContext.MainArena);

				return nullptr;
			}
			default: return CreateEmpty(Operand->Type, CContext.MainArena);
		}
	}

	ExprResult* ExprResult::CastBooleanTo(QualType To, bool Explicit, CompilationContext& CContext)
	{
		if (To == Type)
			return this;

		if (!Explicit)
			return nullptr;

		Arena& MainArena = CContext.MainArena;

		switch (To->GetCategory())
		{
			case TypeCategory::CHAR:
				return CreateChar(To, static_cast<char>(Bool), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, static_cast<Int64>(Bool), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, static_cast<double>(Bool), MainArena);

			default:
				return nullptr;
		}
	}

	ExprResult* ExprResult::CastCharTo(QualType To, bool Explicit, CompilationContext& CContext)
	{
		if (To == Type)
			return this;

		Arena& MainArena = CContext.MainArena;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit ? CreateBool(To, static_cast<bool>(Char), MainArena) : nullptr;

			case TypeCategory::CHAR:
				return CreateChar(To, Char, MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, static_cast<Int64>(Char), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, static_cast<double>(Char), MainArena);

			default:
				return nullptr;
		}
	}

	ExprResult* ExprResult::CastIntegerTo(QualType To, bool Explicit, CompilationContext &CContext)
	{
		if (To == Type)
			return this;

		Arena& MainArena = CContext.MainArena;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit ? CreateBool(To, static_cast<bool>(Int), MainArena) : nullptr;

			case TypeCategory::CHAR:
				return CreateChar(To, static_cast<char>(Int), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, Int, MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, static_cast<double>(Int), MainArena);

			default:
				return nullptr;
		}
	}

	ExprResult* ExprResult::CastFloatTo(QualType To, bool Explicit, CompilationContext &CContext)
	{
		if (To == Type)
			return this;

		Arena& MainArena = CContext.MainArena;

		switch (To->GetCategory())
		{
			case TypeCategory::BOOLEAN:
				return Explicit ? CreateBool(To, static_cast<bool>(Float), MainArena) : nullptr;

			case TypeCategory::CHAR:
				return CreateChar(To, static_cast<char>(Float), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, static_cast<Int64>(Float), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, Float, MainArena);
			default:
				return nullptr;
		}
	}
}
