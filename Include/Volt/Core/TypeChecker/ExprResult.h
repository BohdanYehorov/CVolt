//
// Created by bohdan on 28.01.26.
//

#ifndef CVOLT_ExprResult_H
#define CVOLT_ExprResult_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"

namespace Volt
{
	class ExprResult : public Object
	{
		GENERATED_BODY(ExprResult, Object)
	public:
		static ExprResult* CreateRaw(CompilationContext& CContext);
		static ExprResult* CreateInteger(QualType IntType, Int64 Integer, Arena& MainArena);
		static ExprResult* CreateFloat(QualType FloatType, double Float, Arena& MainArena);
		static ExprResult* CreateBool(QualType BoolType, bool Bool, Arena& MainArena);
		static ExprResult* CreateChar(QualType CharType, char Char, Arena& MainArena);

		static ExprResult* CreateEmpty(QualType Type, Arena& MainArena);

		template <typename T>
		static ExprResult* CreateFromType(QualType Type, T Value, Arena& TypesArena);

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

		static ExprResult* ResolveBinary(ExprResult*& Left, ExprResult*& Right, OperatorType Op,
			CompilationContext& CContext);
		static ExprResult* ResolveUnary(ExprResult*& Operand, OperatorType Op, CompilationContext& CContext);

	private:
		bool CastBooleanTo(TypeCategory To, bool Explicit);
		bool CastCharTo(TypeCategory To, bool Explicit);
		bool CastIntegerTo(TypeCategory To, bool Explicit);
		bool CastFloatTo(TypeCategory To, bool Explicit);

		bool CanCastTo(QualType To, bool Explicit);
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
}

#endif //CVOLT_ExprResult_H