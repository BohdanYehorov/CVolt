//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_DATA_TYPE_H
#define CVOLT_DATA_TYPE_H

#include "ASTNodes.h"
#include <llvm/IR/Type.h>

namespace Volt
{
    class DataType
    {
    public:
        static llvm::Type* ToLLVMPrimitiveType(PrimitiveDataType Type, llvm::LLVMContext& Context);

    private:
        DataTypeNodeBase* Type = nullptr;

    public:
        DataType() = default;
        DataType(DataTypeNodeBase* Type) : Type(Type) {}

        [[nodiscard]] bool IsVoidType() const;
        [[nodiscard]] bool IsIntegerType() const;
        [[nodiscard]] bool IsFloatingPointType() const;

        [[nodiscard]] int GetTypeBitWidth() const;

        [[nodiscard]] PrimitiveDataTypeNode* GetPrimitiveType() const { return Cast<PrimitiveDataTypeNode>(Type); }
        [[nodiscard]] PtrDataTypeNode* GetPtrType() const { return Cast<PtrDataTypeNode>(Type); }
        [[nodiscard]] RefDataTypeNode* GetRefType() const { return Cast<RefDataTypeNode>(Type); }

        llvm::Type* GetLLVMType(llvm::LLVMContext& Context) const { return GetLLVMType(Type, Context); }

    private:
        static llvm::Type* GetLLVMType(const DataTypeNodeBase *Type, llvm::LLVMContext& Context);
    };
}

#endif //CVOLT_DATA_TYPE_H