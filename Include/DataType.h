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

        static std::unordered_map<DataTypeNodeWrap, DataType*, DataTypeHash> CachedTypes;

        static DataType *CachedVoidType;
        static DataType *CachedBoolType;
        static DataType *CachedCharType;
        static DataType *CachedIntegerTypes[4];
        static DataType *CachedFPTypes[4];

    public:
        static DataType *CreateVoid(Arena &TypesArena);
        static DataType *CreateBoolean(Arena &TypesArena);
        static DataType *CreateChar(Arena &TypesArena);
        static DataType *CreateInteger(size_t BitWidth, Arena &TypesArena);
        static DataType *CreateFloatingPoint(size_t BitWidth, Arena &TypesArena);

        static DataType *CreatePtr(DataTypeNodeBase* BaseType, Arena& TypesArena);

        static llvm::Type *ToLLVMPrimitiveType(PrimitiveDataType Type, llvm::LLVMContext &Context);
        static llvm::Type *GetLLVMType(const DataTypeNodeBase *Type, llvm::LLVMContext &Context);
        static bool IsEqual(const DataTypeNodeBase *Left, const DataTypeNodeBase *Right);
        static int GetPrimitiveTypeRank(const PrimitiveDataTypeNode *Type);

    private:
        llvm::LLVMContext *CachedContext = nullptr;
        llvm::Type *CachedType = nullptr;
        DataTypeNodeBase *Type = nullptr;

    public:
        DataType() = default;
        DataType(DataTypeNodeBase *Type) : Type(Type) {}

        [[nodiscard]] VoidTypeNode *GetVoidType() const { return Cast<VoidTypeNode>(Type); }
        [[nodiscard]] BoolTypeNode *GetBooleanType() const { return Cast<BoolTypeNode>(Type); }
        [[nodiscard]] CharTypeNode *GetCharType() const { return Cast<CharTypeNode>(Type); }
        [[nodiscard]] IntegerTypeNode *GetIntegerType() const { return Cast<IntegerTypeNode>(Type); }
        [[nodiscard]] FPTypeNode *GetFloatingPointType() const { return Cast<FPTypeNode>(Type); }

        [[nodiscard]] int GetTypeBitWidth() const;

        [[nodiscard]] DataTypeNodeBase *GetTypeBase() const { return Type; }
        [[nodiscard]] PrimitiveDataTypeNode *GetPrimitiveType() const { return Cast<PrimitiveDataTypeNode>(Type); }
        [[nodiscard]] PtrDataTypeNode *GetPtrType() const { return Cast<PtrDataTypeNode>(Type); }
        [[nodiscard]] RefDataTypeNode *GetRefType() const { return Cast<RefDataTypeNode>(Type); }

        [[nodiscard]] llvm::Type *GetLLVMType(llvm::LLVMContext& Context);
        [[nodiscard]] bool IsEqual(const DataType *Other) const { return IsEqual(Type, Other->Type); }

        [[nodiscard]] int GetPrimitiveTypeRank() const {return GetPrimitiveTypeRank(GetPrimitiveType()); }
    };
}

#endif //CVOLT_DATA_TYPE_H