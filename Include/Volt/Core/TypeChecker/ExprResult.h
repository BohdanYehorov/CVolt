//
// Created by bohdan on 28.01.26.
//

#ifndef CVOLT_ExprResult_H
#define CVOLT_ExprResult_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Core/Memory/AlignedStorage.h"
#include "Volt/Support/ErrorHandling.h"
#include "SemaResult.h"

namespace Volt
{
	class ExprAddress;

	class ExprResult : public SemaResult
	{
		GENERATED_BODY(ExprResult, SemaResult)
	public:
		static ExprResult* CreateRaw(CompilationContext& CContext);
		static ExprResult* CreateInteger(QualType IntType, Int64 Integer, Arena& MainArena);
		static ExprResult* CreateFloat(QualType FloatType, double Float, Arena& MainArena);
		static ExprResult* CreateBool(QualType BoolType, bool Bool, Arena& MainArena);
		static ExprResult* CreateChar(QualType CharType, char Char, Arena& MainArena);
		static ExprResult* CreatePointer(QualType PointerType, ExprAddress* Pointer, Arena& MainArena);

		static ExprResult* CreateEmpty(QualType Type, Arena& MainArena);

		template <typename T>
		static ExprResult* CreateFromType(QualType Type, T Value, Arena& TypesArena);

	private:
		AlignedStorage<Int64, UInt64, double, ExprAddress*> Storage;
		bool bIsEmpty = true;

	public:
		[[nodiscard]] Int64 GetInt() const
		{
			VoltAssert(!bIsEmpty);
			VoltAssert(Type->IsSignedIntegerType());
			return GetValue<Int64>();
		}

		[[nodiscard]] UInt64 GetUInt() const
		{
			VoltAssert(!bIsEmpty);
			VoltAssert(Type->IsUnsignedIntegerType());
			return GetValue<UInt64>();
		}

		[[nodiscard]] double GetFloat() const
		{
			VoltAssert(!bIsEmpty);
			VoltAssert(Type->IsFloatingPointType());
			return GetValue<double>();
		}

		[[nodiscard]] bool GetBool() const
		{
			VoltAssert(!bIsEmpty);
			VoltAssert(Type->IsBoolType());
			return GetValue<bool>();
		}

		[[nodiscard]] char GetChar() const
		{
			VoltAssert(!bIsEmpty);
			VoltAssert(Type->IsCharType());
			return GetValue<char>();
		}

		[[nodiscard]] ExprAddress* GetPointer() const
		{
			VoltAssert(!bIsEmpty);
			VoltAssert(Type->IsPointerType());
			return GetValue<ExprAddress*>();
		}

		[[nodiscard]] bool IsEmpty() const { return bIsEmpty; }

		ExprResult* ImplicitCast(QualType To, CompilationContext& CContext)
		{
			return CastTo(To, false, CContext);
		}

		ExprResult* ExplicitCast(QualType To, CompilationContext& CContext)
		{
			return CastTo(To, true, CContext);
		}

		ExprResult* CastTo(QualType To, bool Explicit, CompilationContext& CContext);

		ExprResult* CreateCmp(ExprResult* Right, OperatorType Op, CompilationContext& CContext) const;

		ExprResult* CreateAdd(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateSub(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateMul(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateDiv(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateMod(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateBitAnd(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateBitOr(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateBitXor(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateBitRShift(ExprResult* Right, CompilationContext& CContext) const;
		ExprResult* CreateBitLShift(ExprResult* Right, CompilationContext& CContext) const;

		ExprResult* CreateNeg(CompilationContext& CContext) const;
		ExprResult* CreateBitNot(CompilationContext& CContext) const;
		ExprResult* CreateNot(CompilationContext& CContext);

	private:
		ExprResult* CastBooleanTo(QualType To, bool Explicit, CompilationContext& CContext);
		ExprResult* CastCharTo(QualType To, bool Explicit, CompilationContext& CContext);
		ExprResult* CastIntegerTo(QualType To, bool Explicit, CompilationContext& CContext);
		ExprResult* CastFloatTo(QualType To, bool Explicit, CompilationContext& CContext);

		template <typename Func>
		ExprResult* CreateBinaryForIntFloat(
			ExprResult* Right, Func Fun, CompilationContext& CContext) const;

		template <typename Func>
		ExprResult* CreateBinaryForInt(
			ExprResult* Right, Func Fun, CompilationContext& CContext) const;

		template <typename T>
		void SetValue(T Value);

		template <typename T>
		T GetValue() const;
	};

	template<typename T>
	ExprResult *ExprResult::CreateFromType(QualType Type, T Value, Arena &TypesArena)
	{
		switch (Type->GetCategory())
		{
			case TypeCategory::INTEGER:
				return CreateInteger(Type, Value, TypesArena);
			case TypeCategory::FLOATING_POINT:
				return CreateFloat(Type, Value, TypesArena);
			case TypeCategory::BOOLEAN:
				return CreateBool(Type, Value, TypesArena);
			case TypeCategory::CHAR:
				return CreateChar(Type, Value, TypesArena);
			default:
				return nullptr;
		}
	}

	template<typename Func>
	ExprResult *ExprResult::CreateBinaryForIntFloat(ExprResult *Right, Func Fun,
		CompilationContext &CContext) const
	{
		if (bIsEmpty)
			VoltUnreachable("Cannot create binary for empty value");

		if (Type != Right->Type)
			VoltUnreachable("Cannot create binary for different types");

		if (auto IntType = Type.CastAs<IntegerType>())
		{
			if (IntType->IsSigned)
				return CreateInteger(Type, Fun(GetInt(), Right->GetInt()), CContext.MainArena);

			return CreateInteger(Type, Fun(GetUInt(), Right->GetUInt()), CContext.MainArena);
		}

		if (Type->IsFloatingPointType())
			return CreateFloat(Type, Fun(GetFloat(), Right->GetFloat()), CContext.MainArena);

		return nullptr;
	}

	template<typename Func>
	ExprResult *ExprResult::CreateBinaryForInt(
		ExprResult *Right, Func Fun, CompilationContext &CContext) const
	{
		if (bIsEmpty)
			VoltUnreachable("Cannot create binary for empty value");

		if (Type != Right->Type)
			VoltUnreachable("Cannot create binary for different types");

		if (auto IntType = Type.CastAs<IntegerType>())
		{
			if (IntType->IsSigned)
				return CreateInteger(Type, Fun(GetInt(), Right->GetInt()), CContext.MainArena);

			return CreateInteger(Type, Fun(GetUInt(), Right->GetUInt()), CContext.MainArena);
		}

		return nullptr;
	}

	template<typename T>
	void ExprResult::SetValue(T Value)
	{
		static_assert(sizeof(T) <= sizeof(Storage));
		std::memcpy(Storage.Buffer, &Value, sizeof(Value));
	}

	template<typename T>
	T ExprResult::GetValue() const
	{
		static_assert(sizeof(T) <= sizeof(Storage));
		T Value;
		std::memcpy(&Value, Storage.Buffer, sizeof(T));
		return Value;
	}
}

#endif //CVOLT_ExprResult_H