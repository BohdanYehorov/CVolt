//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_DATA_TYPE_H
#define CVOLT_DATA_TYPE_H

#include "Volt/Core/AST/ASTNodes.h"
#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/Compiler/Hash/DataTypeHash.h"
#include <llvm/IR/Type.h>

namespace Volt
{
    class DataType : public Object
    {
        GENERATED_BODY(DataType, Object)
    private:
        struct DataTypeNodeWrap
        {
            DataTypeBase* Type;

            DataTypeNodeWrap(DataTypeBase* Type) : Type(Type) {}
            operator DataTypeBase*() const { return Type; }

            bool operator==(const DataTypeNodeWrap& Other) const { return IsEqual(Type, Other.Type); }
        };

        static std::unordered_map<DataTypeNodeWrap, DataType*, DataTypeHash> CachedTypes;

        static DataType *CachedVoidType;
        static DataType *CachedBoolType;
        static DataType *CachedCharType;
        static DataType *CachedIntegerTypes[4];
        static DataType *CachedFPTypes[4];

    public:
        static DataType *Create(DataTypeBase *Base, Arena &TypesArena);
        static DataType *CreateVoid(Arena &TypesArena);
        static DataType *CreateBoolean(Arena &TypesArena);
        static DataType *CreateChar(Arena &TypesArena);
        static DataType *CreateInteger(size_t BitWidth, Arena &TypesArena);
        static DataType *CreateFloatingPoint(size_t BitWidth, Arena &TypesArena);

        static DataType *CreatePtr(DataTypeBase* BaseType, Arena& TypesArena);

        static llvm::Type *GetLLVMType(const DataTypeBase *Type, llvm::LLVMContext &Context);
        static bool IsEqual(const DataTypeBase *Left, const DataTypeBase *Right);
        static int GetPrimitiveTypeRank(const PrimitiveDataType *Type);

    private:
        llvm::LLVMContext *CachedContext = nullptr;
        llvm::Type *CachedType = nullptr;
        DataTypeBase *Type = nullptr;

    public:
        DataType() = default;
        DataType(DataTypeBase *Type) : Type(Type) {}

        [[nodiscard]] VoidType *GetVoidType() const { return Cast<VoidType>(Type); }
        [[nodiscard]] BoolType *GetBooleanType() const { return Cast<BoolType>(Type); }
        [[nodiscard]] CharType *GetCharType() const { return Cast<CharType>(Type); }
        [[nodiscard]] IntegerType *GetIntegerType() const { return Cast<IntegerType>(Type); }
        [[nodiscard]] FloatingPointType *GetFloatingPointType() const { return Cast<FloatingPointType>(Type); }

        [[nodiscard]] int GetTypeBitWidth() const;

        [[nodiscard]] DataTypeBase *GetTypeBase() const { return Type; }
        [[nodiscard]] PrimitiveDataType *GetPrimitiveType() const { return Cast<PrimitiveDataType>(Type); }
        [[nodiscard]] PointerType *GetPtrType() const { return Cast<PointerType>(Type); }
        [[nodiscard]] ReferenceType *GetRefType() const { return Cast<ReferenceType>(Type); }

        [[nodiscard]] llvm::Type *GetLLVMType(llvm::LLVMContext& Context);
        [[nodiscard]] bool IsEqual(const DataType *Other) const { return IsEqual(Type, Other->Type); }

        [[nodiscard]] int GetPrimitiveTypeRank() const {return GetPrimitiveTypeRank(GetPrimitiveType()); }
    };
}

#endif //CVOLT_DATA_TYPE_H