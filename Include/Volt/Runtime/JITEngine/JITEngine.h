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
#include <llvm/ADT/StringMap.h>

namespace Volt
{
	class ClassInst;

	class JITEngine
	{
		std::unique_ptr<llvm::Module> Module;
		BuiltinFunctionTable& BuiltinFuncTable;
		llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> Jit;
		CompilationContext& CContext;
		llvm::StringMap<void*> CachedFunctions;
		llvm::StringMap<void*> CachedMethods;

	public:
		template <typename RetT, typename ... ArgsT>
		using FuncT = RetT(*)(ArgsT...);

	public:
		static void Init();

	public:
		JITEngine(CompilationContext &CContext, BuiltinFunctionTable& Table);

		template <typename RetT, typename ... ArgsT>
		FuncT<RetT, ArgsT...> GetFunctionAddr(const std::string& Name);

		template <typename RetT, typename ... ArgsT>
		RetT CallFunction(const std::string& Name, ArgsT... Args);

		void FillClassMethods(ClassInst& Inst);

	private:
		void* GetRawFuncAddr(const std::string& IRName);
	};

	template<typename RetT, typename ... ArgsT>
	JITEngine::FuncT<RetT, ArgsT...> JITEngine::GetFunctionAddr(const std::string &Name)
	{
		IRNameBuilder NameBuilder(IRNameKind::Function);
		NameBuilder.AddName(Name);

		if constexpr (sizeof...(ArgsT) > 0)
			NameBuilder.AddParams<ArgsT...>(NameBuilder);

		const std::string& IRName = NameBuilder.GetIRName();

		if (auto Iter = CachedFunctions.find(IRName);
			Iter != CachedFunctions.end())
			return reinterpret_cast<FuncT<RetT, ArgsT...>>(Iter->getValue());

		auto SymOrErr = Jit->get()->lookup(IRName);
		if (!SymOrErr)
		{
			llvm::logAllUnhandledErrors(SymOrErr.takeError(), llvm::errs(), "Error: ");
			return nullptr;
		}

		const auto Func = SymOrErr->toPtr<FuncT<RetT, ArgsT...>>();
		CachedFunctions[IRName] = reinterpret_cast<void*>(Func);
		return Func;
	}

	template<typename RetT, typename ... ArgsT>
	RetT JITEngine::CallFunction(const std::string& Name, ArgsT...Args)
	{
		FuncT<RetT, ArgsT...> Func = GetFunctionAddr<RetT, ArgsT...>(Name);
		if (!Func) return RetT();
        return Func(Args...);
	}
}

#endif //CVOLT_JITENGINE_H