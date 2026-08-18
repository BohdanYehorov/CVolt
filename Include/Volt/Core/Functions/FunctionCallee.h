//
// Created by bohdan on 13.02.26.
//

#ifndef CVOLT_FUNCTIONCALLEE_H
#define CVOLT_FUNCTIONCALLEE_H

#include "Callee.h"
#include <llvm/IR/Function.h>

namespace Volt
{
	class FunctionCallee : public CalleeBase
	{
		GENERATED_BODY(FunctionCallee, CalleeBase)
	public:
		llvm::Function* Function = nullptr;
		FunctionCallee(QualType Type)
			: CalleeBase(Type) {}
	};
}

#endif //CVOLT_FUNCTIONCALLEE_H