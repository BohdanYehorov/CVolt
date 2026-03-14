//
// Created by bohdan on 28.01.26.
//

#ifndef CVOLT_CTIMEVALUE_H
#define CVOLT_CTIMEVALUE_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"

namespace Volt
{
	class CTimeValue : public Object
	{
		GENERATED_BODY(CTimeValue, Object)
	public:
		static CTimeValue* CreateRaw(CompilationContext& CContext);
		static CTimeValue* CreateInteger(QualType IntType, Int64 Integer, Arena& MainArena);
		static CTimeValue* CreateFloat(QualType FloatType, double Float, Arena& MainArena);
		static CTimeValue* CreateBool(QualType BoolType, bool Bool, Arena& MainArena);
		static CTimeValue* CreateChar(QualType CharType, char Char, Arena& MainArena);

		static CTimeValue* CreateEmpty(QualType Type, Arena& MainArena);

		template <typename T>
		static CTimeValue* CreateFromType(QualType Type, T Value, Arena& TypesArena);

	public:
		QualType Type;
		union
		{
			Int64 Int;
			double Float;
			bool Bool;
			char Char;
		};
		bool IsEmpty = true;

		bool ImplicitCast(QualType To) { return CastTo(To, false); }
		bool ExplicitCast(QualType To) { return CastTo(To, true); }

		bool CastTo(QualType To, bool Explicit)
		{
			if (CanCastTo(To, Explicit))
			{
				Type = To;
				return true;
			}

			return false;
		}

		static CTimeValue* ResolveBinary(CTimeValue*& Left, CTimeValue*& Right, OperatorType Op,
			CompilationContext& CContext);
		static CTimeValue* ResolveUnary(CTimeValue*& Operand, OperatorType Op, CompilationContext& CContext);

	private:
		bool CastBooleanTo(TypeCategory To, bool Explicit);
		bool CastCharTo(TypeCategory To, bool Explicit);
		bool CastIntegerTo(TypeCategory To, bool Explicit);
		bool CastFloatTo(TypeCategory To, bool Explicit);

		bool CanCastTo(QualType To, bool Explicit);
	};

	template<typename T>
	CTimeValue *CTimeValue::CreateFromType(QualType Type, T Value, Arena &TypesArena)
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
}

#endif //CVOLT_CTIMEVALUE_H