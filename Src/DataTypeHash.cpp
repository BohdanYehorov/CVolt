//
// Created by bohdan on 13.01.26.
//

#include "Volt/Compiler/Hash/DataTypeHash.h"
#include "Volt/Compiler/Types/DataType.h"

namespace Volt
{
    size_t DataTypeHash::operator()(const DataTypeBase *Type) const
    {
        if (!Type)
            throw std::runtime_error("Type is nullptr");

        if (Type->CachedHash != 0)
            return Type->CachedHash;

        if (const auto PrimitiveType = Cast<const PrimitiveDataType>(Type))
            Type->CachedHash = std::hash<int>{}(DataType::GetPrimitiveTypeRank(PrimitiveType));
        else if (const auto PtrType = Cast<const PointerType>(Type))
        {
            size_t Res = operator()(PtrType->BaseType);
            Type->CachedHash = Res + 0x9e3779b9 + (Res << 6) + (Res >> 2);
        }

        return Type->CachedHash;
    }
}