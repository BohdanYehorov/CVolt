//
// Created by bohdan on 28.01.26.
//

#include "Volt/Compiler/CompileTime/CTimeValue.h"

namespace Volt
{
	CTimeValue *CTimeValue::CreateRaw(CompilationContext &CContext)
	{
		return CContext.MainArena.Create<CTimeValue>();
	}

	CTimeValue *CTimeValue::CreateInteger(QualType IntType, Int64 Integer, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = IntType;
		Value->Int = Integer;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateFloat(QualType FloatType, double Float, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = FloatType;
		Value->Float = Float;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateBool(QualType BoolType, bool Bool, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = BoolType;
		Value->Bool = Bool;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateChar(QualType CharType, char Char, Arena &MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = CharType;
		Value->Char = Char;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateEmpty(QualType Type, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = Type;
		Value->IsEmpty = true;
		return Value;
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

	CTimeValue *CTimeValue::ResolveBinary(CTimeValue *&Left, CTimeValue *&Right, OperatorType Op,
		CompilationContext& CContext)
	{
		using enum OperatorType;

		QualType LeftType = Left->Type;
		QualType RightType = Right->Type;

		QualType ResultType = Operator::ResolveBinary(LeftType, RightType, Op, CContext);

		if (!Left->ImplicitCast(LeftType) || !Right->ImplicitCast(RightType))
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

	CTimeValue *CTimeValue::ResolveUnary(CTimeValue *&Operand, OperatorType Op, CompilationContext &CContext)
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
						return CContext.MainArena.Create<CTimeValue>(*Operand);
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
				if (Operand->ImplicitCast(QualType(CContext.GetBoolType(), QualType::CONST)))
					return CreateBool(Operand->Type, !Operand->Bool, CContext.MainArena);

				return nullptr;
			}
			default: return CreateEmpty(Operand->Type, CContext.MainArena);
		}
	}

	bool CTimeValue::CastBooleanTo(TypeCategory To, bool Explicit)
	{
		if (To == TypeCategory::BOOLEAN)
			return true;

		if (!Explicit)
			return false;

		switch (To)
		{
			case TypeCategory::CHAR:
			{
				Char = static_cast<char>(Bool);
				return true;
			}
			case TypeCategory::INTEGER:
			{
				Int = static_cast<Int64>(Bool);
				return true;
			}
			case TypeCategory::FLOATING_POINT:
			{
				Float = static_cast<double>(Bool);
				return true;
			}
			default:
				return false;
		}
	}

	bool CTimeValue::CastCharTo(TypeCategory To, bool Explicit)
	{
		switch (To)
		{
			case TypeCategory::BOOLEAN:
			{
				if (!Explicit)
					return false;

				Bool = static_cast<bool>(Char);
				return true;
			}
			case TypeCategory::CHAR:
				return true;
			case TypeCategory::INTEGER:
			{
				Int = static_cast<Int64>(Char);
				return true;
			}
			case TypeCategory::FLOATING_POINT:
			{
				Float = static_cast<double>(Char);
				return true;
			}
			default:
				return false;
		}
	}

	bool CTimeValue::CastIntegerTo(TypeCategory To, bool Explicit)
	{
		switch (To)
		{
			case TypeCategory::BOOLEAN:
			{
				if (!Explicit)
					return false;

				Bool = static_cast<bool>(Int);
				return true;
			}
			case TypeCategory::CHAR:
			{
				Char = static_cast<char>(Int);
				return true;
			}
			case TypeCategory::INTEGER:
				return true;
			case TypeCategory::FLOATING_POINT:
			{
				Float = static_cast<double>(Int);
				return true;
			}
			default:
				return false;
		}
	}

	bool CTimeValue::CastFloatTo(TypeCategory To, bool Explicit)
	{
		switch (To)
		{
			case TypeCategory::BOOLEAN:
			{
				if (!Explicit)
					return false;

				Bool = static_cast<bool>(Float);
				return true;
			}
			case TypeCategory::CHAR:
			{
				Char = static_cast<char>(Float);
				return true;
			}
			case TypeCategory::INTEGER:
			{
				Int = static_cast<Int64>(Float);
				return true;
			}
			case TypeCategory::FLOATING_POINT:
				return true;
			default:
				return false;
		}
	}

	bool CTimeValue::CanCastTo(QualType To, bool Explicit)
	{
		if (!Type.CastTo(To, Explicit))
			return false;

		if (IsEmpty)
			return true;

		TypeCategory OldTypeCategory = Type->GetCategory();
		TypeCategory NewTypeCategory = To->GetCategory();

		switch (OldTypeCategory)
		{
			case TypeCategory::BOOLEAN:
				return CastBooleanTo(NewTypeCategory, Explicit);

			case TypeCategory::CHAR:
				return CastCharTo(NewTypeCategory, Explicit);

			case TypeCategory::INTEGER:
				return CastIntegerTo(NewTypeCategory, Explicit);

			case TypeCategory::FLOATING_POINT:
				return CastFloatTo(NewTypeCategory, Explicit);

			default:
				return false;
		}
	}
}
