//
// Created by bohdan on 21.01.26.
//

#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"

namespace Volt
{
	void BuiltinFunctionTable::GenSymbolMap(const llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap &SymbolMap)
	{
		for (const auto& [Name, Overload] : Functions)
			SymbolMap[Jit->mangleAndIntern(Overload.Callee->BaseName)] = llvm::orc::ExecutorSymbolDef(
				Overload.ExeAddr, llvm::JITSymbolFlags::Exported);;
	}
}
