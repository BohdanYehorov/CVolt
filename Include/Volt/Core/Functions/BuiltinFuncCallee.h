//
// Created by bohdan on 13.02.26.
//

#ifndef CVOLT_BUILTINFUNCCALLEE_H
#define CVOLT_BUILTINFUNCCALLEE_H

#include "Volt/Core/Types/DataType.h"
#include "Callee.h"
#include <llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h>

namespace Volt
{
	class BuiltinFuncCallee : public FunctionCallee
	{
		GENERATED_BODY(BuiltinFuncCallee, FunctionCallee)
	public:
		std::string BaseName;

		BuiltinFuncCallee(FunctionType* FuncType, llvm::StringRef BaseName)
			: FunctionCallee(FuncType), BaseName(BaseName) {}
	};
}

#endif //CVOLT_BUILTINFUNCCALLEE_H