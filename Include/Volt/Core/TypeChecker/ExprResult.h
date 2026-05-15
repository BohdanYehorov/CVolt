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

		ExprResult* ImplicitCast(QualType To, CompilationContext& CContext)
		{
			return CastTo(To, false, CContext);
		}

		ExprResult* ExplicitCast(QualType To, CompilationContext& CContext)
		{
			return CastTo(To, true, CContext);
		}

		ExprResult* CastTo(QualType To, bool Explicit, CompilationContext& CContext);

		static ExprResult* ResolveBinary(ExprResult*& Left, ExprResult*& Right, OperatorType Op,
			CompilationContext& CContext);
		static ExprResult* ResolveUnary(ExprResult*& Operand, OperatorType Op, CompilationContext& CContext);

	private:
		ExprResult* CastBooleanTo(QualType To, bool Explicit, CompilationContext& CContext);
		ExprResult* CastCharTo(QualType To, bool Explicit, CompilationContext& CContext);
		ExprResult* CastIntegerTo(QualType To, bool Explicit, CompilationContext& CContext);
		ExprResult* CastFloatTo(QualType To, bool Explicit, CompilationContext& CContext);
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