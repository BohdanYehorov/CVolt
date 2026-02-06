//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_DATA_TYPE_H
#define CVOLT_DATA_TYPE_H

#include "Volt/Core/Memory/Arena.h"
#include "Volt/Core/Types/DataType.h"
#include <llvm/IR/Type.h>

namespace Volt
{
    enum class TypeCategory : UInt8
    {
        INVALID,
        VOID,
        BOOLEAN,
        INTEGER,
        FLOATING_POINT,
        POINTER,
        REFERENCE
    };

    class DataTypeUtils
    {
    public:
        static llvm::Type *GetLLVMType(const DataType *Type, llvm::LLVMContext &Context);
        static bool IsEqual(const DataType *Left, const DataType *Right);
        static int GetPrimitiveTypeRank(const PrimitiveDataType *Type);
        static int GetTypeRank(const DataType *Type);
        static std::string TypeToString(DataType* Type);
        static TypeCategory GetTypeCategory(DataType* Type);
    };
}

#endif //CVOLT_DATA_TYPE_H