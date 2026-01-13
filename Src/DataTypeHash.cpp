//
// Created by bohdan on 13.01.26.
//

#include "Volt/Compiler/Hash/DataTypeHash.h"
#include "Volt/Compiler/Types/DataType.h"

namespace Volt
{
    size_t DataTypeHash::operator()(const DataTypeBase *Type) const
    {
        if (const auto PrimitiveType = Cast<const PrimitiveDataType>(Type))
            return std::hash<int>{}(DataType::GetPrimitiveTypeRank(PrimitiveType));

        if (const auto PtrType = Cast<const PointerType>(Type))
        {
            size_t Res = operator()(PtrType->BaseType);
            return Res + 0x9e3779b9 + (Res << 6) + (Res >> 2);
        }

        return 0;
    }
}