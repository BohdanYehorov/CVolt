//
// Created by bohdan on 21.01.26.
//

#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"

namespace Volt
{
	void BuiltinFunctionTable::CreateLLVMFunctions(llvm::Module *Module, llvm::LLVMContext& Context)
	{
		for (const auto& [Name, Overload] : Functions)
		{
			auto* BuiltinCallee = Overload.Callee;
			llvm::Type* RetType = CContext.GetLLVMType(BuiltinCallee->ReturnType.GetType());
			SmallVec8<llvm::Type*> LLVMParams;
			LLVMParams.reserve(Overload.Args.size());
			for (const auto& Param : Overload.Args)
				LLVMParams.push_back(CContext.GetLLVMType(Param.GetType()));

			llvm::FunctionType* FuncType = llvm::FunctionType::get(RetType, LLVMParams, false);
			llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, BuiltinCallee->BaseName, Module);

			BuiltinCallee->SymbolDef = llvm::orc::ExecutorSymbolDef(
				BuiltinCallee->ExeAddr, llvm::JITSymbolFlags::Exported);
		}
	}

	void BuiltinFunctionTable::GenSymbolMap(const llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap &SymbolMap)
	{
		for (const auto& [Name, Overload] : Functions)
			SymbolMap[Jit->mangleAndIntern(Overload.Callee->BaseName)] = Overload.Callee->SymbolDef;
	}
}
