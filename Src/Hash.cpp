//
// Created by bohdan on 08.01.26.
//

#include "Hash.h"
#include "FunctionSignature.h"

namespace Volt
{
    size_t CombineHashes(llvm::ArrayRef<size_t> Hashes)
    {
        size_t Res = 0;

        for (size_t Hash : Hashes)
            Res ^= Hash + 0x9e3779b9 + (Res << 6) + (Res >> 2);

        return Res;
    }

    size_t DataTypeHash::operator()(const DataTypeNodeBase *Type) const
    {
        if (const auto PrimitiveType = Cast<const PrimitiveDataTypeNode>(Type))
            return std::hash<int>{}(static_cast<int>(PrimitiveType->PrimitiveType));

        if (const auto PtrType = Cast<const PtrDataTypeNode>(Type))
        {
            size_t Res = operator()(PtrType->BaseType);
            return Res + 0x9e3779b9 + (Res << 6) + (Res >> 2);
        }

        return 0;
    }

    size_t FunctionSignatureHash::operator()(const FunctionSignature &FuncSign) const
    {
        size_t Seed =  std::hash<std::string>{}(FuncSign.Name);
        for (auto Param : FuncSign.Params)
            CombineHashes(Seed, DataTypeHash{}(Param));

        return Seed;
    }
}