//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_DATA_TYPE_H
#define CVOLT_DATA_TYPE_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Compiler/Hash/DataTypeHash.h"
#include <unordered_set>
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
        static llvm::Type *GetLLVMType(const DataTypeBase *Type, llvm::LLVMContext &Context);
        static bool IsEqual(const DataTypeBase *Left, const DataTypeBase *Right);
        static int GetPrimitiveTypeRank(const PrimitiveDataType *Type);
        static std::string TypeToString(DataTypeBase* Type);
        static TypeCategory GetTypeCategory(DataTypeBase* Type);
    };
}

#endif //CVOLT_DATA_TYPE_H