//
// Created by bohdan on 13.02.26.
//

#ifndef CVOLT_BUILTINFUNCCALLEE_H
#define CVOLT_BUILTINFUNCCALLEE_H

#include "Volt/Core/Types/DataType.h"
#include <llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h>

namespace Volt
{
	class BuiltinFuncCallee : public CalleeBase
	{
		GENERATED_BODY(BuiltinFuncCallee, CalleeBase)
	public:
		std::string BaseName;
		llvm::orc::ExecutorAddr ExeAddr;
		llvm::orc::ExecutorSymbolDef SymbolDef;

		BuiltinFuncCallee(DataType* RetType, const std::string& BaseName, llvm::orc::ExecutorAddr ExeAddr)
			: CalleeBase(RetType), BaseName(BaseName), ExeAddr(ExeAddr) {}
	};
}

#endif //CVOLT_BUILTINFUNCCALLEE_H