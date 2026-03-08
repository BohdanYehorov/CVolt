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

	CTimeValue *CTimeValue::CreateInteger(DataType* IntType, Int64 Integer, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = IntType;
		Value->Int = Integer;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateFloat(DataType* FloatType, double Float, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = FloatType;
		Value->Float = Float;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateBool(DataType* BoolType, bool Bool, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = BoolType;
		Value->Bool = Bool;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateChar(DataType* CharType, char Char, Arena &MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = CharType;
		Value->Char = Char;
		Value->IsEmpty = false;
		return Value;
	}

	CTimeValue *CTimeValue::CreateEmpty(DataType* Type, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = Type;
		Value->IsEmpty = true;
		return Value;
	}

	bool CTimeValue::ImplicitCast(DataType *To)
	{
		if (Type == To)
			return true;

		DataType* NewType = Type->ImplicitCast(To);
		if (!NewType)
			return false;

		if (IsEmpty)
		{
			Type = NewType;
			return true;
		}

		TypeCategory OldTypeCategory = Type->GetCategory();
		TypeCategory NewTypeCategory = NewType->GetCategory();

		switch (OldTypeCategory)
		{
			case TypeCategory::BOOLEAN:
			{
				switch (NewTypeCategory)
				{
					case TypeCategory::BOOLEAN:
						return true;
					case TypeCategory::CHAR:
						Char = static_cast<char>(Bool);
						Type = NewType;
						return true;
					case TypeCategory::INTEGER:
						Int = static_cast<Int64>(Bool);
						Type = NewType;
						return true;
					default:
						return false;
				}
			}

			case TypeCategory::CHAR:
			{
				switch (NewTypeCategory)
				{
					case TypeCategory::BOOLEAN:
						Bool = static_cast<bool>(Char);
						Type = NewType;
						return true;
					case TypeCategory::CHAR:
						return true;
					case TypeCategory::INTEGER:
						Int = static_cast<Int64>(Char);
						Type = NewType;
						return true;
					case TypeCategory::FLOATING_POINT:
						Float = static_cast<double>(Char);
						Type = NewType;
						return true;
					default:
						return false;
				}
			}

			case TypeCategory::INTEGER:
			{
				switch (NewTypeCategory)
				{
					case TypeCategory::BOOLEAN:
						Bool = static_cast<bool>(Int);
						Type = NewType;
						return true;
					case TypeCategory::CHAR:
						Char = static_cast<char>(Int);
						Type = NewType;
						return true;
					case TypeCategory::INTEGER:
						Type = NewType;
						return true;
					case TypeCategory::FLOATING_POINT:
						Float = static_cast<double>(Int);
						Type = NewType;
						return true;
					default:
						return false;
				}
			}

			case TypeCategory::FLOATING_POINT:
			{
				switch (NewTypeCategory)
				{
					case TypeCategory::CHAR:
						Char = static_cast<char>(Float);
						Type = NewType;
						return true;
					case TypeCategory::INTEGER:
						Int = static_cast<Int64>(Float);
						Type = NewType;
						return true;
					case TypeCategory::FLOATING_POINT:
						Type = NewType;
						return true;
					default:
						return false;
				}
			}

			default:
				return false;
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

	CTimeValue *CTimeValue::ResolveBinary(CTimeValue *&Left, CTimeValue *&Right, OperatorType Op,
		CompilationContext& CContext)
	{
		using enum OperatorType;

		DataType* LeftType = Left->Type;
		DataType* RightType = Right->Type;

		DataType* ResultType = Operator::ResolveBinary(LeftType, RightType, Op, CContext);

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
			if (auto Type = Operator::ResolveUnary(Operand->Type, Op))
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
				if (Operand->ImplicitCast(CContext.GetBoolType()))
					return CreateBool(Operand->Type, !Operand->Bool, CContext.MainArena);

				return nullptr;
			}
			default: return CreateEmpty(Operand->Type, CContext.MainArena);
		}
	}
}
