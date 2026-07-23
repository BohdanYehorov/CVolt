//
// Created by bohdan on 14.02.26.
//

#include "Volt/Runtime/JITEngine/JITEngine.h"
#include <llvm/Support/TargetSelect.h>

namespace Volt
{
	void JITEngine::Init()
	{
		static bool JITInit = false;
		if (!JITInit)
		{
			llvm::InitializeNativeTarget();
			llvm::InitializeNativeTargetAsmPrinter();
			llvm::InitializeNativeTargetAsmParser();
			JITInit = true;
		}
	}

	JITEngine::JITEngine(CompilationContext &CContext, BuiltinFunctionTable &Table)
		: Module(std::move(CContext.Module)), BuiltinFuncTable(Table),
		Jit(llvm::orc::LLJITBuilder().create()), CContext(CContext)
	{
		if (!Jit)
		{
			llvm::errs() << "Failed to create JIT: " << Jit.takeError();
			return;
		}

		llvm::orc::SymbolMap Symbols;

		BuiltinFuncTable.GenSymbolMap(Jit->get(), Symbols);

		cantFail(Jit->get()->getMainJITDylib().define(
			llvm::orc::absoluteSymbols(Symbols)
		));

		auto NewContext = std::make_unique<llvm::LLVMContext>();

		llvm::orc::ThreadSafeModule TSM{std::move(Module), std::move(NewContext)};

		if (auto Err = Jit->get()->addIRModule(std::move(TSM)))
			llvm::errs() << Err;
	}
}
