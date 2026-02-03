//
// Created by bohdan on 28.01.26.
//

#ifndef CVOLT_CTIMEVALUE_H
#define CVOLT_CTIMEVALUE_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Compiler/Types/DataType.h"

namespace Volt
{
	class CTimeValue : public Object
	{
		GENERATED_BODY(CTimeValue, Object)
	public:
		static CTimeValue* CreateInteger(DataTypeBase* IntType, Int64 Integer, Arena& MainArena);
		static CTimeValue* CreateFloat(DataTypeBase* FloatType, double Float, Arena& MainArena);
		static CTimeValue* CreateBool(DataTypeBase* BoolType, bool Bool, Arena& MainArena);
		static CTimeValue* CreateChar(DataTypeBase* CharType, char Char, Arena& MainArena);
		static CTimeValue* CreatePointer(DataTypeBase* PtrType, void* Ptr, Arena& MainArena);
		static CTimeValue* CreateNull(DataTypeBase* Type, Arena& MainArena);

	public:
		DataTypeBase* Type;
		union
		{
			Int64 Int;
			double Float;
			bool Bool;
			char Char;
			void* Ptr;
		};
		bool IsValid = true;

		operator bool() const { return IsValid; }
	};
}

#endif //CVOLT_CTIMEVALUE_H