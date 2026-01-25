//
// Created by bohdan on 13.01.26.
//

#include "Volt/Compiler/Hash/DataTypeHash.h"

#include "Volt/Compiler/Hash/Hash.h"
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
            Type->CachedHash = std::hash<size_t>{}(PrimitiveType->Object_GetType());
        else if (const auto ArrType = Cast<const ArrayType>(Type))
        {
            size_t Res = std::hash<size_t>{}(ArrType->Object_GetType());
            CombineHashes(Res, std::hash<size_t>{}(ArrType->Length));
            CombineHashes(Res, operator()(ArrType->BaseType));
            Type->CachedHash = Res;
        }
        else if (const auto PtrType = Cast<const PointerType>(Type))
        {
            size_t Res = std::hash<size_t>{}(PtrType->Object_GetType());
            CombineHashes(Res, operator()(PtrType->BaseType));
            Type->CachedHash = Res;
        }

        return Type->CachedHash;
    }
}