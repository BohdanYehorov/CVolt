//
// Created by bohdan on 21.01.26.
//

#ifndef CVOLT_BUILTINFUNCTIONTABLE_H
#define CVOLT_BUILTINFUNCTIONTABLE_H
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Functions/FunctionSignature.h"
#include "Volt/Core/Hash/Hash.h"
#include "Volt/Core/Types/TypeConv.h"
#include "Volt/Core/Functions/BuiltinFuncCallee.h"
#include "Volt/Utils/IRNameBuilder.h"
#include <llvm/ExecutionEngine/Orc/CoreContainers.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

namespace Volt
{
	class BuiltinFunctionTable
	{
	public:
		// using Map = std::unordered_map<FunctionSignature, BuiltinFuncCallee*, Hash<FunctionSignature>>;

	private:
		FunctionMap Functions;
		CompilationContext& CContext;
		Arena& MainArena;

	public:
		BuiltinFunctionTable(CompilationContext& CContext)
			: CContext(CContext), MainArena(CContext.MainArena) {}

		template <typename Ret, typename ...Args>
		void AddFunction(llvm::StringRef Name, Ret(*FuncPtr)(Args...));

		template <typename Ret>
		void AddFunction(llvm::StringRef Name, Ret(*FuncPtr)());

		template <typename T, typename ...ArgsTy>
		void AddFunctionOverloads(llvm::StringRef Name, T Fun, ArgsTy... Args);

		void CreateLLVMFunctions(llvm::Module *Module, llvm::LLVMContext& Context);
		void GenSymbolMap(const llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap& SymbolMap);

		[[nodiscard]] const FunctionMap& GetMap() const { return Functions; }

	private:
		template <typename T, typename ...Rest>
		void FillParams(ArgsVector<QualType>& Params);
	};

	template<typename T, typename ... Rest>
	void BuiltinFunctionTable::FillParams(ArgsVector<QualType> &Params)
	{
		Params.push_back(TypeConv::GetDataType<T>(CContext));
		if constexpr (sizeof...(Rest) > 0)
			FillParams<Rest...>(Params);
	}

	template<typename Ret, typename ... Args>
	void BuiltinFunctionTable::AddFunction(llvm::StringRef Name, Ret(*FuncPtr)(Args...))
	{
		QualType RetType = TypeConv::GetDataType<Ret>(CContext);
		ArgsVector<QualType> Params;
		FillParams<Args...>(Params);
		IRNameBuilder NameBuilder(IRNameKind::Function);
		NameBuilder.AddName(Name);
		for (const auto& Param : Params)
			NameBuilder.AddParam(Param);

		auto* Callee = MainArena.Create<BuiltinFuncCallee>(
			RetType, NameBuilder.GetIRName(), llvm::orc::ExecutorAddr::fromPtr(FuncPtr));

		Functions[Name].emplace_back(std::move(Params), Callee);
	}

	template<typename Ret>
	void BuiltinFunctionTable::AddFunction(llvm::StringRef Name, Ret(*FuncPtr)())
	{
		QualType RetType = TypeConv::GetDataType<Ret>(CContext);
		IRNameBuilder NameBuilder(IRNameKind::Function);
		NameBuilder.AddName(Name);

		auto* Callee = MainArena.Create<BuiltinFuncCallee>(
			RetType, NameBuilder.GetIRName(), llvm::orc::ExecutorAddr::fromPtr(FuncPtr));

		Functions[Name].emplace_back(ArgsVector<QualType>(), Callee);
	}

	template<typename T, typename ... ArgsTy>
	void BuiltinFunctionTable::AddFunctionOverloads(llvm::StringRef Name, T Fun, ArgsTy... Args)
	{
		static_assert(std::is_pointer_v<T>);
		static_assert(std::is_function_v<std::remove_pointer_t<T>>);
		AddFunction(Name, Fun);
		if constexpr (sizeof...(ArgsTy) > 0)
			AddFunctionOverloads(Name, Args...);
	}
}

#endif //CVOLT_BUILTINFUNCTIONTABLE_H