//
// Created by bohdan on 21.01.26.
//

#ifndef CVOLT_BUILTINFUNCTIONTABLE_H
#define CVOLT_BUILTINFUNCTIONTABLE_H
#include "Volt/Core/Types/DataTypeUtils.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Compiler/Hash/FunctionSignatureHash.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h>
#include <llvm/ExecutionEngine/Orc/CoreContainers.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

namespace Volt
{
	struct FunctionData
	{
		DataType* ReturnType;
		std::string BaseName;
		llvm::orc::ExecutorAddr ExeAddr;
		llvm::orc::ExecutorSymbolDef SymbolDef;
	};

	class BuiltinFunctionTable
	{
	private:
		std::unordered_map<FunctionSignature, FunctionData, FunctionSignatureHash> Functions;
		CompilationContext& CContext;

	public:
		BuiltinFunctionTable(CompilationContext& CContext) : CContext(CContext) {}

		template <typename Ret, typename ...Args>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)(Args...));

		template <typename Ret>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)());

		void CreateLLVMFunctions(llvm::Module *Module, llvm::LLVMContext& Context);
		void GenSymbolMap(llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap& SymbolMap);

		FunctionData* Get(const FunctionSignature& Signature);

	private:
		template <typename T, typename ...Rest>
		void FillParams(SmallVec8<DataType*>& Params);
	};

	template <typename T>
	DataType* GetBaseType(CompilationContext& CContext);

	template<> inline DataType* GetBaseType<void>(CompilationContext& CContext)
	{
		return CContext.GetVoidType();
	}

	template<> inline DataType* GetBaseType<bool>(CompilationContext& CContext)
	{
		return CContext.GetBoolType();
	}

	template<> inline DataType* GetBaseType<char>(CompilationContext& CContext)
	{
		return CContext.GetCharType();
	}

	template<> inline DataType* GetBaseType<std::byte>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(8);
	}

	template<> inline DataType* GetBaseType<short>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(16);
	}

	template<> inline DataType* GetBaseType<int>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(32);
	}

	template<> inline DataType* GetBaseType<long>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(64);
	}

	template<> inline DataType* GetBaseType<long long>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(64);
	}

	template<> inline DataType* GetBaseType<float>(CompilationContext& CContext)
	{
		return CContext.GetFPType(32);
	}

	template<> inline DataType* GetBaseType<double>(CompilationContext& CContext)
	{
		return CContext.GetFPType(64);
	}

	template <typename T>
	DataType* GetDataType(CompilationContext& CContext)
	{
		if constexpr (std::is_pointer_v<T>)
		{
			using BaseType = std::remove_pointer_t<T>;
			return CContext.GetPointerType(GetDataType<BaseType>(CContext));
		}

		return GetBaseType<T>(CContext);
	}

	template<typename T, typename ... Rest>
	void BuiltinFunctionTable::FillParams(SmallVec8<DataType*> &Params)
	{
		Params.push_back(GetDataType<T>(CContext));
		if constexpr (sizeof...(Rest) > 0)
			FillParams<Rest...>(Params);
	}

	template<typename Ret, typename ... Args>
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)(Args...))
	{
		DataType* RetType = GetDataType<Ret>(CContext);
		SmallVec8<DataType*> Params;
		FillParams<Args...>(Params);
		FunctionSignature Signature{ Name, Params };
		Functions[Signature] = { RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr) };
	}

	template<typename Ret>
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)())
	{
		DataType* RetType = GetDataType<Ret>(CContext);
		FunctionSignature Signature{ Name, {} };
		Functions[Signature] = { RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr) };
	}
}

#endif //CVOLT_BUILTINFUNCTIONTABLE_H