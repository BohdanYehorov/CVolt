//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_DATATYPEBASE_H
#define CVOLT_DATATYPEBASE_H

#include <llvm/ADT/Hashing.h>

#include "Volt/Core/Object/Object.h"
#include <llvm/IR/Type.h>

namespace Volt
{
    class DataTypeBase : public Object
    {
        GENERATED_BODY(DataTypeBase, Object)
    private:
        mutable size_t CachedHash = 0;
        llvm::Type* CachedType = nullptr;
        friend class DataTypeHash;
        friend class DataType;
    };

    class PrimitiveDataType : public DataTypeBase
    {
        GENERATED_BODY(PrimitiveDataType, DataTypeBase)
    };

    class VoidType : public PrimitiveDataType
    {
        GENERATED_BODY(VoidType, PrimitiveDataType)
    };

    class BoolType : public PrimitiveDataType
    {
        GENERATED_BODY(BoolType, PrimitiveDataType)
    };

    class CharType : public PrimitiveDataType
    {
        GENERATED_BODY(CharType, PrimitiveDataType)
    };

    class IntegerType : public PrimitiveDataType
    {
        GENERATED_BODY(IntegerType, PrimitiveDataType)
    public:
        size_t BitWidth;
        bool IsSigned;
        IntegerType(size_t BitWidth, bool IsSigned = false)
            : BitWidth(BitWidth), IsSigned(IsSigned) {}
    };

    class FloatingPointType : public PrimitiveDataType
    {
        GENERATED_BODY(FloatingPointType, PrimitiveDataType)
    public:
        size_t BitWidth;
        FloatingPointType(size_t BitWidth) : BitWidth(BitWidth) {}
    };

    class PointerType : public DataTypeBase
    {
        GENERATED_BODY(PointerType, DataTypeBase)
    public:
        DataTypeBase* BaseType;
        PointerType(DataTypeBase* BaseType)
            : BaseType(BaseType) {}
    };

    class ArrayType : public PointerType
    {
        GENERATED_BODY(ArrayType, PointerType)
    public:
        size_t Length;

        ArrayType(DataTypeBase* BaseType, size_t Length)
            : PointerType(BaseType), Length(Length) {}
    };

    class ReferenceType : public DataTypeBase
    {
        GENERATED_BODY(ReferenceType, DataTypeBase)
    public:
        DataTypeBase* BaseType;
        ReferenceType(DataTypeBase* BaseType)
            : BaseType(BaseType) {}
    };
}

#endif //CVOLT_DATATYPEBASE_H