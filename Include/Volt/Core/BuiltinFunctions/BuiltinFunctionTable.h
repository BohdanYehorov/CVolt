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
#include <llvm/ExecutionEngine/Orc/CoreContainers.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

namespace Volt
{
	class BuiltinFunctionTable
	{
	public:
		using Map = std::unordered_map<FunctionSignature, BuiltinFuncCallee*, Hash<FunctionSignature>>;

	private:
		Map Functions;
		CompilationContext& CContext;
		Arena& MainArena;

	public:
		BuiltinFunctionTable(CompilationContext& CContext)
			: CContext(CContext), MainArena(CContext.MainArena) {}

		template <typename Ret, typename ...Args>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)(Args...));

		template <typename Ret>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)());

		void CreateLLVMFunctions(llvm::Module *Module, llvm::LLVMContext& Context);
		void GenSymbolMap(const llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap& SymbolMap);

		[[nodiscard]] BuiltinFuncCallee* Get(const FunctionSignature& Signature);

		[[nodiscard]] const Map& GetMap() const { return Functions; }

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
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)(Args...))
	{
		QualType RetType = TypeConv::GetDataType<Ret>(CContext);
		ArgsVector<QualType> Params;
		FillParams<Args...>(Params);
		FunctionSignature Signature{ Name, std::move(Params) };
		Functions[Signature] = MainArena.Create<BuiltinFuncCallee>(
			RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr));
	}

	template<typename Ret>
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)())
	{
		QualType RetType = TypeConv::GetDataType<Ret>(CContext);
		FunctionSignature Signature{ Name, {} };
		Functions[Signature] = MainArena.Create<BuiltinFuncCallee>(
			RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr));
	}
}

#endif //CVOLT_BUILTINFUNCTIONTABLE_H