//
// Created by bohdan on 03.02.26.
//

#ifndef CVOLT_CALLEE_H
#define CVOLT_CALLEE_H
#include "Volt/Core/Object/Object.h"

namespace Volt
{
	class CalleeBase : public Object
	{
		GENERATED_BODY(CalleeBase, Object)

	public:
		DataType ReturnType;
		CalleeBase(DataType ReturnType)
			: ReturnType(ReturnType) {}
	};

	class FunctionCallee : public CalleeBase
	{
		GENERATED_BODY(FunctionCallee, CalleeBase);

	public:
		FunctionCallee(DataType ReturnType)
			: CalleeBase(ReturnType) {}
	};
}

#endif //CVOLT_CALLEE_H