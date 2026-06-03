//
// Created by bohdan on 11.03.26.
//

#ifndef CVOLT_HASH_H
#define CVOLT_HASH_H

#include <cstddef>
#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Functions/FunctionSignature.h"

namespace Volt
{
	inline void CombineHashes(size_t& Seed, size_t Hash)
	{
		Seed ^= Hash + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
	}

	template <typename T>
	class Hash
	{
	public:
		size_t operator()(const T& Value)
		{
			return std::hash<T>{}(Value);
		}
	};

	template <>
	class Hash<DataType*>
	{
	public:
		size_t operator()(const DataType* Type) const
		{
			return Type->GetHash();
		}
	};

	template <>
	class Hash<QualType>
	{
	public:
		size_t operator()(const QualType& Type) const
		{
			size_t H = Type->GetHash();
			CombineHashes(H, Hash<UInt32>{}(Type.GetQuals()));
			return H;
		}
	};

	template <>
	class Hash<FunctionSignature>
	{
	public:
		size_t operator()(const FunctionSignature& Signature) const
		{
			size_t Seed =  std::hash<std::string>{}(Signature.Name);
			for (auto Param : Signature.Params)
				CombineHashes(Seed, Param.GetHash());

			return Seed;
		}
	};
}

#endif //CVOLT_HASH_H