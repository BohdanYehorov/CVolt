//
// Created by bohdan on 14.02.26.
//

#ifndef CVOLT_JITENGINE_H
#define CVOLT_JITENGINE_H

#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Core/BuiltinFunctions/BuiltinFunctionTable.h"
#include "Volt/Utils/IRNameBuilder.h"
#include "Volt/Core/Types/ClassInst.h"
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ADT/StringMap.h>

namespace Volt
{
	class JITEngine
	{
		std::unique_ptr<llvm::Module> Module;
		BuiltinFunctionTable& BuiltinFuncTable;
		llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> Jit;
		CompilationContext& CContext;
		llvm::StringMap<void*> CachedFunctions;
		llvm::StringMap<void*> CachedMethods;

	public:
		static void Init();

	public:
		JITEngine(CompilationContext &CContext, BuiltinFunctionTable& Table);

		template <typename RetT, typename ... ArgsT>
		RetT CallFunction(const std::string& Name, ArgsT... Args);

		template <typename RetT, typename ... ArgsT>
		RetT CallMethod(ClassInst& Inst, const std::string& Name, ArgsT ... Args);

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

		const std::string& IRName = NameBuilder.GetIRName();

		using FuncT = RetT(*)(ArgsT...);

		if (auto Iter = CachedFunctions.find(IRName);
			Iter != CachedFunctions.end())
			return reinterpret_cast<FuncT>(Iter->getValue())(Args...);

        auto SymOrErr = Jit->get()->lookup(IRName);
        if (!SymOrErr)
        {
            llvm::logAllUnhandledErrors(SymOrErr.takeError(), llvm::errs(), "Error: ");
            return RetT();
        }

		const auto Func = SymOrErr->toPtr<FuncT>();
		CachedFunctions[IRName] = reinterpret_cast<void*>(Func);
        return Func(Args...);
	}

	template<typename RetT, typename ... ArgsT>
	RetT JITEngine::CallMethod(ClassInst &Inst, const std::string &Name, ArgsT... Args)
	{
		IRNameBuilder NameBuilder(IRNameKind::Method);
		NameBuilder.AddName(Inst.GetType()->Name);
		NameBuilder.AddName(Name);

		if constexpr (sizeof...(ArgsT) > 0)
			AddParams<ArgsT...>(NameBuilder);

		const std::string& IRName = NameBuilder.GetIRName();

		using MethodT = RetT(*)(void*, ArgsT...);
		if (auto Iter = CachedMethods.find(IRName);
			Iter != CachedMethods.end())
			return reinterpret_cast<MethodT>(Iter->getValue())(Inst.GetData(), Args...);

		auto SymOrErr = Jit->get()->lookup(IRName);
		if (!SymOrErr)
		{
			llvm::logAllUnhandledErrors(SymOrErr.takeError(), llvm::errs(), "Error: ");
			return RetT();
		}

		const auto Func = SymOrErr->toPtr<MethodT>();
		CachedMethods[IRName] = reinterpret_cast<void*>(Func);
		return Func(Inst.GetData(), Args...);
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