//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_DATATYPEBASE_H
#define CVOLT_DATATYPEBASE_H

#include "Volt/Core/Object/Object.h"

namespace Volt
{
    class DataTypeBase : public Object
    {
        GENERATED_BODY(DataTypeNodeBase, Object)
    };

    class PrimitiveDataType : public DataTypeBase
    {
        GENERATED_BODY(PrimitiveDataTypeNode, DataTypeBase)
    };

    class VoidType : public PrimitiveDataType
    {
        GENERATED_BODY(VoidTypeNode, PrimitiveDataType)
    };

    class BoolType : public PrimitiveDataType
    {
        GENERATED_BODY(BoolTypeNode, PrimitiveDataType)
    };

    class CharType : public PrimitiveDataType
    {
        GENERATED_BODY(CharTypeNode, PrimitiveDataType)
    };

    class IntegerType : public PrimitiveDataType
    {
        GENERATED_BODY(IntegerTypeNode, PrimitiveDataType)
    public:
        size_t BitWidth;
        bool IsSigned;
        IntegerType(size_t BitWidth, bool IsSigned = false)
            : BitWidth(BitWidth), IsSigned(IsSigned) {}
    };

    class FloatingPointType : public PrimitiveDataType
    {
        GENERATED_BODY(FPTypeNode, PrimitiveDataType)
    public:
        size_t BitWidth;
        FloatingPointType(size_t BitWidth) : BitWidth(BitWidth) {}
    };

    class PointerType : public DataTypeBase
    {
        GENERATED_BODY(PtrDataTypeNode, DataTypeBase)
    public:
        DataTypeBase* BaseType;
        PointerType(DataTypeBase* BaseType)
            : BaseType(BaseType) {}
    };

    class ReferenceType : public DataTypeBase
    {
        GENERATED_BODY(RefDataTypeNode, DataTypeBase)
    public:
        DataTypeBase* BaseType;
        ReferenceType(DataTypeBase* BaseType)
            : BaseType(BaseType) {}
    };
}

#endif //CVOLT_DATATYPEBASE_H