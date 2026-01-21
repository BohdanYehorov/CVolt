//
// Created by bohdan on 21.01.26.
//

#ifndef CVOLT_BUILTINFUNCTIONTABLE_H
#define CVOLT_BUILTINFUNCTIONTABLE_H
#include "Volt/Compiler/Types/DataType.h"
#include "Volt/Compiler/Functions/FunctionSignature.h"
#include "Volt/Compiler/Hash/FunctionSignatureHash.h"
#include <llvm/IR/Module.h>
#include <llvm/ExecutionEngine/Orc/Shared/ExecutorSymbolDef.h>
#include <llvm/ExecutionEngine/Orc/CoreContainers.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

namespace Volt
{
	struct FunctionData
	{
		DataType ReturnType;
		std::string BaseName;
		llvm::orc::ExecutorAddr ExeAddr;
		llvm::orc::ExecutorSymbolDef SymbolDef;
	};

	class BuiltinFunctionTable
	{
	private:
		std::unordered_map<FunctionSignature, FunctionData, FunctionSignatureHash> Functions;
		Arena& TypesArena;

	public:
		BuiltinFunctionTable(Arena& TypesArena) : TypesArena(TypesArena) {}

		template <typename Ret, typename ...Args>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)(Args...));

		template <typename Ret>
		void AddFunction(const std::string& Name, const std::string& BaseName, Ret(*FuncPtr)());

		void CreateLLVMFunctions(llvm::Module *Module);
		void GenSymbolMap(llvm::orc::LLJIT *Jit, llvm::orc::SymbolMap& SymbolMap);

		FunctionData* Get(const FunctionSignature& Signature);

	private:
		template <typename T, typename ...Rest>
		void FillParams(llvm::SmallVector<DataType, 8>& Params);
	};

	template <typename T>
	DataType GetBaseType(Arena& TypesArena);

	template<> inline DataType GetBaseType<void>(Arena& TypesArena)
	{
		return DataType::CreateVoid(TypesArena);
	}

	template<> inline DataType GetBaseType<bool>(Arena& TypesArena)
	{
		return DataType::CreateBoolean(TypesArena);
	}

	template<> inline DataType GetBaseType<char>(Arena& TypesArena)
	{
		return DataType::CreateChar(TypesArena);
	}

	template<> inline DataType GetBaseType<std::byte>(Arena& TypesArena)
	{
		return DataType::CreateInteger(8, TypesArena);
	}

	template<> inline DataType GetBaseType<short>(Arena& TypesArena)
	{
		return DataType::CreateInteger(16, TypesArena);
	}

	template<> inline DataType GetBaseType<int>(Arena& TypesArena)
	{
		return DataType::CreateInteger(32, TypesArena);
	}

	template<> inline DataType GetBaseType<long>(Arena& TypesArena)
	{
		return DataType::CreateInteger(64, TypesArena);
	}

	template<> inline DataType GetBaseType<long long>(Arena& TypesArena)
	{
		return DataType::CreateInteger(64, TypesArena);
	}

	template<> inline DataType GetBaseType<float>(Arena& TypesArena)
	{
		return DataType::CreateFloatingPoint(32, TypesArena);
	}

	template<> inline DataType GetBaseType<double>(Arena& TypesArena)
	{
		return DataType::CreateFloatingPoint(64, TypesArena);
	}

	template <typename T>
	DataType GetDataType(Arena& TypesArena)
	{
		if constexpr (std::is_pointer_v<T>)
		{
			using BaseType = std::remove_pointer_t<T>;
			return DataType::CreatePtr(GetDataType<BaseType>(TypesArena), TypesArena);
		}

		return GetBaseType<T>(TypesArena);
	}

	template<typename T, typename ... Rest>
	void BuiltinFunctionTable::FillParams(llvm::SmallVector<DataType, 8> &Params)
	{
		Params.push_back(GetDataType<T>(TypesArena));
		if constexpr (sizeof...(Rest) > 0)
			FillParams<Rest...>(Params);
	}

	template<typename Ret, typename ... Args>
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)(Args...))
	{
		DataType RetType = GetDataType<Ret>(TypesArena);
		llvm::SmallVector<DataType, 8> Params;
		FillParams<Args...>(Params);
		FunctionSignature Signature{ Name, Params };
		Functions[Signature] = { RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr) };
	}

	template<typename Ret>
	void BuiltinFunctionTable::AddFunction(const std::string &Name, const std::string &BaseName, Ret(*FuncPtr)())
	{
		DataType RetType = GetDataType<Ret>(TypesArena);
		FunctionSignature Signature{ Name, {} };
		Functions[Signature] = { RetType, BaseName, llvm::orc::ExecutorAddr::fromPtr(FuncPtr) };
	}
}

#endif //CVOLT_BUILTINFUNCTIONTABLE_H