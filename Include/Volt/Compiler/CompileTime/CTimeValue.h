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
		static CTimeValue* CreateInteger(DataType* IntType, Int64 Integer, Arena& MainArena);
		static CTimeValue* CreateFloat(DataType* FloatType, double Float, Arena& MainArena);
		static CTimeValue* CreateBool(DataType* BoolType, bool Bool, Arena& MainArena);
		static CTimeValue* CreateChar(DataType* CharType, char Char, Arena& MainArena);

		static CTimeValue* CreateEmpty(DataType* Type, Arena& MainArena);

		template <typename T>
		static CTimeValue* CreateFromType(DataType* Type, T Value, Arena& TypesArena);

	public:
		DataType* Type;
		union
		{
			Int64 Int;
			double Float;
			bool Bool;
			char Char;
		};
		bool IsEmpty = true;

		bool ImplicitCast(DataType* To);
		static CTimeValue* ResolveBinary(CTimeValue*& Left, CTimeValue*& Right, OperatorType Op,
			CompilationContext& CContext);
		static CTimeValue* ResolveUnary(CTimeValue*& Operand, OperatorType Op, CompilationContext& CContext);
	};

	template<typename T>
	CTimeValue *CTimeValue::CreateFromType(DataType *Type, T Value, Arena &TypesArena)
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