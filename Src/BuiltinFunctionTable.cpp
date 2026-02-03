//
// Created by bohdan on 21.01.26.
//

#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"

namespace Volt
{
	void BuiltinFunctionTable::CreateLLVMFunctions(llvm::Module *Module, llvm::LLVMContext& Context)
	{
		for (auto& [Signature, Data] : Functions)
		{
			llvm::Type* RetType = DataType::GetLLVMType(Data.ReturnType, Context);
			llvm::SmallVector<llvm::Type*, 8> LLVMParams;
			LLVMParams.reserve(Signature.Params.size());
			for (const auto& Param : Signature.Params)
				LLVMParams.push_back(DataType::GetLLVMType(Param, Context));

			llvm::FunctionType* FuncType = llvm::FunctionType::get(RetType, LLVMParams, false);
			llvm::Function::Create(FuncType, llvm::Function::ExternalLinkage, Data.BaseName, Module);

			Data.SymbolDef = llvm::orc::ExecutorSymbolDef(
			    Data.ExeAddr, llvm::JITSymbolFlags::Exported);
		}
	}

	void BuiltinFunctionTable::GenSymbolMap(llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap &SymbolMap)
	{
		for (const auto& [Signature, Data] : Functions)
			SymbolMap[Jit->mangleAndIntern(Data.BaseName)] = Data.SymbolDef;
	}

	FunctionData * BuiltinFunctionTable::Get(const FunctionSignature &Signature)
	{
		if (auto Iter = Functions.find(Signature); Iter != Functions.end())
			return &Iter->second;
		return nullptr;
	}
}
