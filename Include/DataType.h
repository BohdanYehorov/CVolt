//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_DATA_TYPE_H
#define CVOLT_DATA_TYPE_H

#include "ASTNodes.h"
#include <llvm/IR/Type.h>
#include "Object.h"
#include "Arena.h"
#include "Hash.h"

namespace Volt
{
    class DataType : public Object
    {
        GENERATED_BODY(DataType, Object)
    private:
        struct DataTypeNodeWrap
        {
            DataTypeNodeBase* Type;

            DataTypeNodeWrap(DataTypeNodeBase* Type) : Type(Type) {}
            operator DataTypeNodeBase*() const { return Type; }

            bool operator==(const DataTypeNodeWrap& Other) const { return IsEqual(Type, Other.Type); }
        };

        static std::unordered_map<PrimitiveDataType, DataType*> CachedPrimitiveTypes;
        static std::unordered_map<DataTypeNodeWrap, DataType*, DataTypeHash> CachedTypes;

    public:
        static DataType* CreatePrimitive(PrimitiveDataType Type, Arena& TypesArena);
        static DataType* CreatePtr(DataTypeNodeBase* BaseType, Arena& TypesArena);

        static llvm::Type* ToLLVMPrimitiveType(PrimitiveDataType Type, llvm::LLVMContext& Context);
        static llvm::Type* GetLLVMType(const DataTypeNodeBase *Type, llvm::LLVMContext& Context);
        static bool IsEqual(const DataTypeNodeBase* Left, const DataTypeNodeBase* Right);

    private:
        DataTypeNodeBase* Type = nullptr;

    public:
        DataType() = default;
        DataType(DataTypeNodeBase* Type) : Type(Type) {}

        [[nodiscard]] bool IsVoidType() const;
        [[nodiscard]] bool IsBooleanType() const;
        [[nodiscard]] bool IsIntegerType() const;
        [[nodiscard]] bool IsFloatingPointType() const;

        [[nodiscard]] int GetTypeBitWidth() const;

        [[nodiscard]] DataTypeNodeBase* GetTypeBase() const { return Type; }
        [[nodiscard]] PrimitiveDataTypeNode* GetPrimitiveType() const { return Cast<PrimitiveDataTypeNode>(Type); }
        [[nodiscard]] PtrDataTypeNode* GetPtrType() const { return Cast<PtrDataTypeNode>(Type); }
        [[nodiscard]] RefDataTypeNode* GetRefType() const { return Cast<RefDataTypeNode>(Type); }

        [[nodiscard]] llvm::Type* GetLLVMType(llvm::LLVMContext& Context) const;
        [[nodiscard]] bool IsEqual(const DataType* Other) const { return IsEqual(Type, Other->Type); }

        [[nodiscard]] int GetPrimitiveTypeRank() const;
    };
}

#endif //CVOLT_DATA_TYPE_H