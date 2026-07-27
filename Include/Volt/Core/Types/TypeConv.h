//
// Created by bohdan on 06.03.26.
//

#ifndef CVOLT_TYPECONV_H
#define CVOLT_TYPECONV_H

#include "DataType.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"

namespace Volt
{
	template <typename>
	class VoltClass;
}

namespace Volt::TypeConv
{
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

	template<> inline DataType* GetBaseType<Int8>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(8);
	}

	template<> inline DataType* GetBaseType<Int16>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(16);
	}

	template<> inline DataType* GetBaseType<Int32>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(32);
	}

	template<> inline DataType* GetBaseType<Int64>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(64);
	}

	template<> inline DataType* GetBaseType<UInt8>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(8, false);
	}

	template<> inline DataType* GetBaseType<UInt16>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(16, false);
	}

	template<> inline DataType* GetBaseType<UInt32>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(32, false);
	}

	template<> inline DataType* GetBaseType<UInt64>(CompilationContext& CContext)
	{
		return CContext.GetIntegerType(64, false);
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
	QualType GetDataType(CompilationContext& CContext)
	{
		if constexpr (std::is_const_v<T>)
		{
			using BaseType = std::remove_const_t<T>;
			return { GetDataType<BaseType>(CContext).GetType(), QualType::CONST };
		}

		if constexpr (std::is_pointer_v<T>)
		{
			using BaseType = std::remove_pointer_t<T>;
			return { CContext.GetPointerType(GetDataType<BaseType>(CContext)), 0 };
		}

		if constexpr (std::is_reference_v<T>)
		{
			using BaseType = std::remove_reference_t<T>;
			return { CContext.GetReferenceType(GetDataType<BaseType>(CContext)), 0 };
		}

		if constexpr (std::is_base_of_v<VoltClass<T>, T>)
			return  { T::GetClassType(), 0 };

		return { GetBaseType<T>(CContext), 0 };
	}
}

#endif //CVOLT_TYPECONV_H