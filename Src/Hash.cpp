//
// Created by bohdan on 08.01.26.
//

#include "../Include/Hash.h"

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