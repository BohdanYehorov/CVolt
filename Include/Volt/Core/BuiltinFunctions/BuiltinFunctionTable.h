//
// Created by bohdan on 21.01.26.
//

#ifndef CVOLT_BUILTINFUNCTIONTABLE_H
#define CVOLT_BUILTINFUNCTIONTABLE_H
#include "Volt/Compiler/Types/DataTypeUtils.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Compiler/Hash/FunctionSignatureHash.h"
#include "Volt/Core/Builder/Builder.h"
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
		BuilderBase& Builder;

	public:
		BuiltinFunctionTable(BuilderBase& Builder) : Builder(Builder) {}

		template <typename Ret, typename ...Args>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)(Args...));

		template <typename Ret>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)());

		void CreateLLVMFunctions(llvm::Module *Module, llvm::LLVMContext& Context);
		void GenSymbolMap(llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap& SymbolMap);

		FunctionData* Get(const FunctionSignature& Signature);

	private:
		template <typename T, typename ...Rest>
		void FillParams(llvm::SmallVector<DataType*, 8>& Params);
	};

	template <typename T>
	DataType* GetBaseType(BuilderBase& Builder);

	template<> inline DataType* GetBaseType<void>(BuilderBase& Builder)
	{
		return Builder.GetVoidType();
	}

	template<> inline DataType* GetBaseType<bool>(BuilderBase& Builder)
	{
		return Builder.GetBoolType();
	}

	template<> inline DataType* GetBaseType<char>(BuilderBase& Builder)
	{
		return Builder.GetCharType();
	}

	template<> inline DataType* GetBaseType<std::byte>(BuilderBase& Builder)
	{
		return Builder.GetIntegerType(8);
	}

	template<> inline DataType* GetBaseType<short>(BuilderBase& Builder)
	{
		return Builder.GetIntegerType(16);
	}

	template<> inline DataType* GetBaseType<int>(BuilderBase& Builder)
	{
		return Builder.GetIntegerType(32);
	}

	template<> inline DataType* GetBaseType<long>(BuilderBase& Builder)
	{
		return Builder.GetIntegerType(64);
	}

	template<> inline DataType* GetBaseType<long long>(BuilderBase& Builder)
	{
		return Builder.GetIntegerType(64);
	}

	template<> inline DataType* GetBaseType<float>(BuilderBase& Builder)
	{
		return Builder.GetFPType(32);
	}

	template<> inline DataType* GetBaseType<double>(BuilderBase& Builder)
	{
		return Builder.GetFPType(64);
	}

	template <typename T>
	DataType* GetDataType(BuilderBase& Builder)
	{
		if constexpr (std::is_pointer_v<T>)
		{
			using BaseType = std::remove_pointer_t<T>;
			return Builder.GetPointerType(GetDataType<BaseType>(Builder));
		}

		return GetBaseType<T>(Builder);
	}

	template<typename T, typename ... Rest>
	void BuiltinFunctionTable::FillParams(llvm::SmallVector<DataType*, 8> &Params)
	{
		Params.push_back(GetDataType<T>(Builder));
		if constexpr (sizeof...(Rest) > 0)
			FillParams<Rest...>(Params);
	}

	template<typename Ret, typename ... Args>
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)(Args...))
	{
		DataType* RetType = GetDataType<Ret>(Builder);
		llvm::SmallVector<DataType*, 8> Params;
		FillParams<Args...>(Params);
		FunctionSignature Signature{ Name, Params };
		Functions[Signature] = { RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr) };
	}

	template<typename Ret>
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)())
	{
		DataType* RetType = GetDataType<Ret>(Builder);
		FunctionSignature Signature{ Name, {} };
		Functions[Signature] = { RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr) };
	}
}

#endif //CVOLT_BUILTINFUNCTIONTABLE_H