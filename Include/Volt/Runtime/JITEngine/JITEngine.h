//
// Created by bohdan on 14.02.26.
//

#ifndef CVOLT_JITENGINE_H
#define CVOLT_JITENGINE_H

#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

namespace Volt
{
	class JITEngine
	{
		std::unique_ptr<llvm::Module> Module;
		BuiltinFunctionTable& BuiltinFuncTable;
		llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> Jit;

	public:
		static void Init();

	public:
		JITEngine(CompilationContext &CContext, BuiltinFunctionTable& Table);

		template <typename RetT, typename ... ArgsT>
		RetT CallFunction(const std::string& Name, ArgsT... Args);
	};

	template<typename RetT, typename ... ArgsT>
	RetT JITEngine::CallFunction(const std::string& Name, ArgsT...Args)
	{
        auto SymOrErr = Jit->get()->lookup(Name);
        if (!SymOrErr)
        {
            llvm::logAllUnhandledErrors(SymOrErr.takeError(), llvm::errs(), "Error: ");
            return 1;
        }

		const auto Func = SymOrErr->toPtr<RetT(*)(ArgsT...)>();
        return Func(Args...);
	}
}

#endif //CVOLT_JITENGINE_H