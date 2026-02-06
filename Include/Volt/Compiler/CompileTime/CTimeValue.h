//
// Created by bohdan on 28.01.26.
//

#ifndef CVOLT_CTIMEVALUE_H
#define CVOLT_CTIMEVALUE_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Types/DataTypeUtils.h"

namespace Volt
{
	class CTimeValue : public Object
	{
		GENERATED_BODY(CTimeValue, Object)
	public:
		static CTimeValue* CreateInteger(DataType* IntType, Int64 Integer, Arena& MainArena);
		static CTimeValue* CreateFloat(DataType* FloatType, double Float, Arena& MainArena);
		static CTimeValue* CreateBool(DataType* BoolType, bool Bool, Arena& MainArena);
		static CTimeValue* CreateChar(DataType* CharType, char Char, Arena& MainArena);
		static CTimeValue* CreatePointer(DataType* PtrType, void* Ptr, Arena& MainArena);
		static CTimeValue* CreateNull(DataType* Type, Arena& MainArena);

	public:
		DataType* Type;
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