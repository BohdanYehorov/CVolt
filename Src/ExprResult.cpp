//
// Created by bohdan on 28.01.26.
//

#include "Volt/Core/TypeChecker/ExprResult.h"
#include "Volt/Core/TypeChecker/ExprAddress.h"

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
		Value->SetValue(Integer);
		Value->bIsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateFloat(QualType FloatType, double Float, Arena& MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = FloatType;
		Value->SetValue(Float);
		Value->bIsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateBool(QualType BoolType, bool Bool, Arena& MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = BoolType;
		Value->SetValue(Bool);
		Value->bIsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateChar(QualType CharType, char Char, Arena &MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = CharType;
		Value->SetValue(Char);
		Value->bIsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreatePointer(QualType PointerType, ExprAddress *Pointer, Arena &MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = PointerType;
		Value->SetValue(Pointer);
		Value->bIsEmpty = false;
		return Value;
	}

	ExprResult *ExprResult::CreateEmpty(QualType Type, Arena& MainArena)
	{
		auto Value = MainArena.Create<ExprResult>();
		Value->Type = Type;
		Value->bIsEmpty = true;
		return Value;
	}

	ExprResult *ExprResult::CastTo(QualType To, bool Explicit, CompilationContext &CContext)
	{
		if (Type == To)
			return this;

		if (!Type.CastTo(To, Explicit))
			return nullptr;

		if (bIsEmpty)
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

	ExprResult *ExprResult::CreateAdd(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
			[](auto A, auto B) { return A + B; },
			[](auto A, auto B) { return A + B; },
			CContext);

	}

	ExprResult *ExprResult::CreateSub(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
			[](auto A, auto B) { return A - B; },
			[](auto A, auto B) { return A - B; },
			CContext);
	}

	ExprResult *ExprResult::CreateMul(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
			[](auto A, auto B) { return A * B; },
			[](auto A, auto B) { return A * B; },
			CContext);
	}

	ExprResult *ExprResult::CreateDiv(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
			[](auto A, auto B) { return A / B; },
			[](auto A, auto B) { return A / B; },
			CContext);
	}

	ExprResult *ExprResult::CreateMod(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForInt(Right,
			[](auto A, auto B) { return A % B; },
			CContext);
	}

	ExprResult *ExprResult::CreateBitAnd(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForInt(Right,
			[](auto A, auto B) { return A & B; },
			CContext);
	}

	ExprResult *ExprResult::CreateBitOr(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForInt(Right,
			[](auto A, auto B) { return A | B; },
			CContext);
	}

	ExprResult *ExprResult::CreateBitXor(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForInt(Right,
			[](auto A, auto B) { return A ^ B; },
			CContext);
	}

	ExprResult *ExprResult::CreateBitRShift(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForInt(Right,
			[](auto A, auto B) { return A >> B; },
			CContext);
	}

	ExprResult *ExprResult::CreateBitLShift(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForInt(Right,
			[](auto A, auto B) { return A << B; },
			CContext);
	}

	ExprResult* ExprResult::CreateNeg(CompilationContext &CContext) const
	{
		if (bIsEmpty)
			llvm_unreachable("Cannot create unary for empty value");

		if (Type->IsIntegerType())
			return CreateInteger(Type, -GetInt(), CContext.MainArena);

		if (Type->IsFloatingPointType())
			return CreateFloat(Type, -GetFloat(), CContext.MainArena);

		return nullptr;
	}

	ExprResult* ExprResult::CreateBitNot(CompilationContext &CContext) const
	{
		if (bIsEmpty)
			llvm_unreachable("Cannot create unary for empty value");

		if (Type->IsIntegerType())
			return CreateInteger(Type, ~GetInt(), CContext.MainArena);

		return nullptr;
	}

	ExprResult* ExprResult::CreateNot(CompilationContext &CContext)
	{
		DataType* BoolTy = CContext.GetBoolType();
		ExprResult* BoolValue = ImplicitCast(BoolTy, CContext);
		if (!BoolValue) return nullptr;

		if (BoolValue == this)
			return CreateBool(BoolTy, !GetBool(), CContext.MainArena);

		BoolValue->SetValue(!BoolValue->GetBool());
		return BoolValue;
	}

	// #define RESOLVE_OP_FOR_ALL_TYPES(Op) switch (LeftType->GetCategory()) \
// { \
// case TypeCategory::INTEGER: \
// return CreateFromType(ResultType, Left->Int Op Right->Int, CContext.MainArena); \
// case TypeCategory::FLOATING_POINT: \
// return CreateFromType(ResultType, Left->Float Op Right->Float, CContext.MainArena); \
// case TypeCategory::BOOLEAN: \
// return CreateFromType(ResultType, Left->Bool Op Right->Bool, CContext.MainArena); \
// case TypeCategory::CHAR: \
// return CreateFromType(ResultType, Left->Char Op Right->Char, CContext.MainArena); \
// default: \
// return nullptr; \
// }
//
// #define RESOLVE_OP_FOR_INT_FLOAT_TYPES(Op) switch (LeftType->GetCategory()) \
// { \
// case TypeCategory::INTEGER: \
// return CreateFromType(ResultType, Left->Int Op Right->Int, CContext.MainArena); \
// case TypeCategory::FLOATING_POINT: \
// return CreateFromType(ResultType, Left->Float Op Right->Float, CContext.MainArena); \
// case TypeCategory::CHAR: \
// return CreateFromType(ResultType, Left->Char Op Right->Char, CContext.MainArena); \
// default: \
// return nullptr; \
// }
//
// #define RESOLVE_OP_FOR_INT_TYPES(Op) {switch (LeftType->GetCategory()) \
// { \
// case TypeCategory::INTEGER: \
// return CreateFromType(ResultType, Left->Int Op Right->Int, CContext.MainArena); \
// case TypeCategory::CHAR: \
// return CreateFromType(ResultType, Left->Char Op Right->Char, CContext.MainArena); \
// default: \
// return nullptr; \
// }}
//
// 	ExprResult *ExprResult::ResolveBinary(ExprResult *&Left, ExprResult *&Right, OperatorType Op,
// 										  CompilationContext& CContext)
// 	{
// 		using enum OperatorType;
//
// 		QualType LeftType = Left->Type;
// 		QualType RightType = Right->Type;
//
// 		QualType ResultType = Operator::ResolveBinary(LeftType, RightType, Op, CContext);
//
// 		Left = Left->ImplicitCast(LeftType, CContext);
// 		Right = Right->ImplicitCast(RightType, CContext);
//
// 		if (!Left || !Right)
// 			return nullptr;
//
// 		if (Left->bIsEmpty || Right->bIsEmpty)
// 			return CreateEmpty(ResultType, CContext.MainArena);
//
// 		switch (Op)
// 		{
// 			case ADD: RESOLVE_OP_FOR_INT_FLOAT_TYPES(+)
// 			case SUB: RESOLVE_OP_FOR_INT_FLOAT_TYPES(-)
// 			case MUL: RESOLVE_OP_FOR_INT_FLOAT_TYPES(*)
// 			case DIV: RESOLVE_OP_FOR_INT_FLOAT_TYPES(/)
// 			case MOD: RESOLVE_OP_FOR_INT_TYPES(%)
// 			case BIT_AND: RESOLVE_OP_FOR_INT_TYPES(&)
// 			case BIT_OR: RESOLVE_OP_FOR_INT_TYPES(|)
// 			case BIT_XOR: RESOLVE_OP_FOR_INT_TYPES(^)
// 			case RSHIFT: RESOLVE_OP_FOR_INT_TYPES(>>)
// 			case LSHIFT: RESOLVE_OP_FOR_INT_TYPES(<<)
//
// 			case EQ: RESOLVE_OP_FOR_ALL_TYPES(==)
// 			case NEQ: RESOLVE_OP_FOR_ALL_TYPES(!=)
//
// 			case GT: RESOLVE_OP_FOR_INT_FLOAT_TYPES(>)
// 			case GTE: RESOLVE_OP_FOR_INT_FLOAT_TYPES(>=)
// 			case LT: RESOLVE_OP_FOR_INT_FLOAT_TYPES(<)
// 			case LTE: RESOLVE_OP_FOR_INT_FLOAT_TYPES(<=)
//
// 			default: return nullptr;
// 		}
// 	}
//
// 	ExprResult *ExprResult::ResolveUnary(ExprResult *&Operand, OperatorType Op, CompilationContext &CContext)
// 	{
// 		using enum TypeCategory;
//
// 		if (!Operand)
// 			return nullptr;
//
// 		if (Operand->bIsEmpty)
// 		{
// 			QualType OperandType = Operand->Type;
// 			if (auto Type = Operator::ResolveUnary(OperandType, Op))
// 				return CreateEmpty(Type, CContext.MainArena);
//
// 			return nullptr;
// 		}
//
// 		TypeCategory OperandTypeCategory = Operand->Type->GetCategory();
// 		switch (Op)
// 		{
// 			case OperatorType::ADD:
// 			{
// 				switch (OperandTypeCategory)
// 				{
// 					case CHAR:
// 					case INTEGER:
// 					case FLOATING_POINT:
// 						return CContext.MainArena.Create<ExprResult>(*Operand);
// 					default:
// 						return nullptr;
// 				}
// 			}
// 			case OperatorType::SUB:
// 			{
// 				switch (OperandTypeCategory)
// 				{
// 					case CHAR:
// 						return CreateChar(Operand->Type, -Operand->Char, CContext.MainArena);
// 					case INTEGER:
// 						return CreateInteger(Operand->Type, -Operand->Int, CContext.MainArena);
// 					case FLOATING_POINT:
// 						return CreateFloat(Operand->Type, -Operand->Float, CContext.MainArena);
// 					default:
// 						return nullptr;
// 				}
// 			}
// 			case OperatorType::BIT_NOT:
// 			{
// 				switch (OperandTypeCategory)
// 				{
// 					case CHAR:
// 						return CreateChar(Operand->Type, ~Operand->Char, CContext.MainArena);
// 					case INTEGER:
// 						return CreateInteger(Operand->Type, ~Operand->Int, CContext.MainArena);
// 					default:
// 						return nullptr;
// 				}
// 			}
// 			case OperatorType::LOGICAL_NOT:
// 			{
// 				if (auto CastedOp = Operand->ImplicitCast(
// 					QualType(CContext.GetBoolType(), QualType::CONST), CContext))
// 					return CreateBool(CastedOp->Type, !CastedOp->Bool, CContext.MainArena);
//
// 				return nullptr;
// 			}
// 			default: return CreateEmpty(Operand->Type, CContext.MainArena);
// 		}
// 	}

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
				return CreateChar(To, static_cast<char>(GetBool()), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, static_cast<Int64>(GetBool()), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, static_cast<double>(GetBool()), MainArena);

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
				return Explicit ? CreateBool(To, static_cast<bool>(GetChar()), MainArena) : nullptr;

			case TypeCategory::CHAR:
				return CreateChar(To, GetChar(), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, static_cast<Int64>(GetChar()), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, static_cast<double>(GetChar()), MainArena);

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
				return Explicit ? CreateBool(To, static_cast<bool>(GetInt()), MainArena) : nullptr;

			case TypeCategory::CHAR:
				return CreateChar(To, static_cast<char>(GetInt()), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, GetInt(), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, static_cast<double>(GetInt()), MainArena);

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
				return Explicit ? CreateBool(To, static_cast<bool>(GetFloat()), MainArena) : nullptr;

			case TypeCategory::CHAR:
				return CreateChar(To, static_cast<char>(GetFloat()), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, static_cast<Int64>(GetFloat()), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, GetFloat(), MainArena);
			default:
				return nullptr;
		}
	}
}