//
// Created by bohdan on 03.02.26.
//

#ifndef CVOLT_CALLEE_H
#define CVOLT_CALLEE_H
#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Types/DataType.h"

namespace Volt
{
	class CalleeBase : public Object
	{
		GENERATED_BODY(CalleeBase, Object)
	public:
		DataType* ReturnType;
		CalleeBase(DataType* ReturnType)
			: ReturnType(ReturnType) {}
	};
}

#endif //CVOLT_CALLEE_H