//
// Created by bohdan on 14.02.26.
//

#ifndef CVOLT_JITENGINE_H
#define CVOLT_JITENGINE_H

#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Utils/IRNameBuilder.h"
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
		CompilationContext& CContext;

	public:
		static void Init();

	public:
		JITEngine(CompilationContext &CContext, BuiltinFunctionTable& Table);

		template <typename RetT, typename ... ArgsT>
		RetT CallFunction(const std::string& Name, ArgsT... Args);

	private:
		template <typename T, typename ... ArgsT>
		void AddParams(IRNameBuilder& NameBuilder);
	};

	template<typename RetT, typename ... ArgsT>
	RetT JITEngine::CallFunction(const std::string& Name, ArgsT...Args)
	{
		IRNameBuilder NameBuilder(IRNameKind::Function);
		NameBuilder.AddName(Name);
		
		if constexpr (sizeof...(ArgsT) > 0)
			AddParams<ArgsT...>(NameBuilder);

        auto SymOrErr = Jit->get()->lookup(NameBuilder.GetIRName());
        if (!SymOrErr)
        {
            llvm::logAllUnhandledErrors(SymOrErr.takeError(), llvm::errs(), "Error: ");
            return RetT();
        }

		const auto Func = SymOrErr->toPtr<RetT(*)(ArgsT...)>();
        return Func(Args...);
	}

	template<typename T, typename ... ArgsT>
	void JITEngine::AddParams(IRNameBuilder &NameBuilder)
	{
		NameBuilder.AddParam(TypeConv::GetDataType<T>(CContext));
		if constexpr (sizeof...(ArgsT) > 0)
			AddParams<ArgsT...>(NameBuilder);
	}
}

#endif //CVOLT_JITENGINE_H