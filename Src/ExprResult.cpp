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
		VoltAssert(IntType->IsIntegerType());

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

	ExprResult *ExprResult::CreateCmp(ExprResult *Right, OperatorType Op, CompilationContext &CContext) const
	{
		using enum OperatorType;

		if (bIsEmpty || Right->bIsEmpty)
			VoltUnreachable("Cannot compare empty values");

		if (Type != Right->Type)
			VoltUnreachable("Cannot compare values with different types");

		if (Type->IsIntegerType())
		{
			Int64 LeftVal  = GetInt();
			Int64 RightVal = Right->GetInt();
			bool Result;

			switch (Op)
			{
				case EQ:  Result = LeftVal == RightVal; break;
				case NEQ: Result = LeftVal != RightVal; break;
				case LT:  Result = LeftVal < RightVal;  break;
				case LTE: Result = LeftVal <= RightVal; break;
				case GT:  Result = LeftVal > RightVal;  break;
				case GTE: Result = LeftVal >= RightVal; break;
				default: VoltUnreachable("Unknown comparison operator");
			}

			return CreateBool(CContext.GetBoolType(), Result, CContext.MainArena);
		}

		if (Type->IsFloatingPointType())
		{
			double LeftVal  = GetFloat();
			double RightVal = Right->GetFloat();
			bool Result;

			switch (Op)
			{
				case EQ:  Result = LeftVal == RightVal; break;
				case NEQ: Result = LeftVal != RightVal; break;
				case LT:  Result = LeftVal < RightVal;  break;
				case LTE: Result = LeftVal <= RightVal; break;
				case GT:  Result = LeftVal > RightVal;  break;
				case GTE: Result = LeftVal >= RightVal; break;
				default: VoltUnreachable("Unknown comparison operator");
			}

			return CreateBool(CContext.GetBoolType(), Result, CContext.MainArena);
		}

		if (Type->IsCharType() || Type->IsBoolType())
		{
			auto LeftVal  = static_cast<Int32>(Type->IsCharType() ? GetChar() : GetBool());
			auto RightVal = static_cast<Int32>(Type->IsCharType() ? Right->GetChar() : Right->GetBool());
			bool Result;

			switch (Op)
			{
				case EQ:  Result = LeftVal == RightVal; break;
				case NEQ: Result = LeftVal != RightVal; break;
				default:  VoltUnreachable("Invalid comparison operator for this type");
			}

			return CreateBool(CContext.GetBoolType(), Result, CContext.MainArena);
		}

		VoltUnreachable("Cannot compare this type");
	}

	ExprResult *ExprResult::CreateAdd(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
			[](auto A, auto B) { return A + B; },
			CContext);

	}

	ExprResult *ExprResult::CreateSub(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
			[](auto A, auto B) { return A - B; },
			CContext);
	}

	ExprResult *ExprResult::CreateMul(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
			[](auto A, auto B) { return A * B; },
			CContext);
	}

	ExprResult *ExprResult::CreateDiv(ExprResult *Right, CompilationContext &CContext) const
	{
		return CreateBinaryForIntFloat(Right,
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
			VoltUnreachable("Cannot create unary for empty value");

		if (Type->IsSignedIntegerType())
			return CreateInteger(Type, -GetInt(), CContext.MainArena);

		if (Type->IsFloatingPointType())
			return CreateFloat(Type, -GetFloat(), CContext.MainArena);

		return nullptr;
	}

	ExprResult* ExprResult::CreateBitNot(CompilationContext &CContext) const
	{
		if (bIsEmpty)
			VoltUnreachable("Cannot create unary for empty value");

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
				return Explicit ? CreateBool(To, static_cast<bool>(GetValue<Int64>()), MainArena) : nullptr;

			case TypeCategory::CHAR:
				return CreateChar(To, static_cast<char>(GetValue<Int64>()), MainArena);

			case TypeCategory::INTEGER:
				return CreateInteger(To, GetValue<Int64>(), MainArena);

			case TypeCategory::FLOATING_POINT:
				return CreateFloat(To, static_cast<double>(GetValue<Int64>()), MainArena);

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