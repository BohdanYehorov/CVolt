//
// Created by bohdan on 07.02.26.
//

#ifndef CVOLT_DEBUGOUTPUT_H
#define CVOLT_DEBUGOUTPUT_H

#include <ostream>
#include <llvm/ADT/APFloat.h>

namespace Volt
{
	class DebugOutput
	{
		std::ostream& Os;
		llvm::raw_ostream& LLVMOs;

		DebugOutput(std::ostream& Os, llvm::raw_ostream& LLVMOs) : Os(Os), LLVMOs(LLVMOs) {}
	};
}

#endif //CVOLT_DEBUGOUTPUT_H