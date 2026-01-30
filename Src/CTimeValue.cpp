//
// Created by bohdan on 28.01.26.
//

#include "Volt/Compiler/CompileTime/CTimeValue.h"

namespace Volt
{
	CTimeValue *CTimeValue::CreateInteger(DataType IntType, Int64 Integer, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = IntType;
		Value->Int = Integer;
		return Value;
	}

	CTimeValue *CTimeValue::CreateFloat(DataType FloatType, double Float, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = FloatType;
		Value->Float = Float;
		return Value;
	}

	CTimeValue *CTimeValue::CreateBool(DataType BoolType, bool Bool, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = BoolType;
		Value->Bool = Bool;
		return Value;
	}

	CTimeValue *CTimeValue::CreateChar(DataType CharType, char Char, Arena &MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = CharType;
		Value->Char = Char;
		return Value;
	}

	CTimeValue *CTimeValue::CreatePointer(DataType PtrType, void *Ptr, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = PtrType;
		Value->Ptr = Ptr;
		return Value;
	}

	CTimeValue *CTimeValue::CreateNull(DataType Type, Arena& MainArena)
	{
		auto Value = MainArena.Create<CTimeValue>();
		Value->Type = Type;
		Value->IsValid = false;
		return Value;
	}
}