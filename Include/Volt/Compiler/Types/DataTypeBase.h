//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_DATATYPEBASE_H
#define CVOLT_DATATYPEBASE_H

#include "Volt/Core/Object/Object.h"
#include <llvm/IR/Type.h>

namespace Volt
{
    class DataType : public Object
    {
        GENERATED_BODY(DataTypeBase, Object)
    private:
        mutable size_t CachedHash = 0;
        llvm::Type* CachedType = nullptr;
        friend class DataTypeHash;
        friend class DataType;
    };

    class PrimitiveDataType : public DataType
    {
        GENERATED_BODY(PrimitiveDataType, DataType)
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

    class PointerType : public DataType
    {
        GENERATED_BODY(PointerType, DataType)
    public:
        DataType* BaseType;
        PointerType(DataType* BaseType)
            : BaseType(BaseType) {}
    };

    class ArrayType : public DataType
    {
        GENERATED_BODY(ArrayType, DataType)
    public:
        DataType* BaseType;
        size_t Length;
        bool LengthInit;

        ArrayType(DataType* BaseType, size_t Length)
            : BaseType(BaseType), Length(Length), LengthInit(true) {}
        ArrayType(DataType* BaseType)
            : BaseType(BaseType), Length(0), LengthInit(false) {}
    };

    class ReferenceType : public DataType
    {
        GENERATED_BODY(ReferenceType, DataType)
    public:
        DataType* BaseType;
        ReferenceType(DataType* BaseType)
            : BaseType(BaseType) {}
    };
}

#endif //CVOLT_DATATYPEBASE_H