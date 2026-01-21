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

    class DataType final
    {
    private:
        struct DataTypeNodeWrap
        {
            DataTypeBase* Type;

            DataTypeNodeWrap(DataTypeBase* Type) : Type(Type) {}
            operator DataTypeBase*() const { return Type; }

            bool operator==(const DataTypeNodeWrap& Other) const { return IsEqual(Type, Other.Type); }
        };

        static std::unordered_map<DataTypeNodeWrap, DataType, DataTypeHash> CachedTypes;

        static DataTypeBase* CachedVoidType;
        static DataTypeBase* CachedBoolType;
        static DataTypeBase* CachedCharType;
        static DataTypeBase* CachedIntegerTypes[4];
        static DataTypeBase* CachedFPTypes[4];

        static llvm::LLVMContext* CachedContext;
        static llvm::LLVMContext* Context;

    public:
        static DataTypeBase* CreateVoid(Arena &TypesArena);
        static DataTypeBase* CreateBoolean(Arena &TypesArena);
        static DataTypeBase* CreateChar(Arena &TypesArena);
        static DataTypeBase* CreateInteger(size_t BitWidth, Arena &TypesArena);
        static DataTypeBase* CreateFloatingPoint(size_t BitWidth, Arena &TypesArena);

        static DataTypeBase* CreatePtr(DataTypeBase* BaseType, Arena& TypesArena);

        static llvm::Type *GetLLVMType(const DataTypeBase *Type, llvm::LLVMContext &Context);
        static bool IsEqual(const DataTypeBase *Left, const DataTypeBase *Right);
        static int GetPrimitiveTypeRank(const PrimitiveDataType *Type);
        static int GetTypeRank(const DataTypeBase* Type, Arena &TypesArena);

    private:
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

        [[nodiscard]] llvm::Type *GetLLVMType(llvm::LLVMContext& TmpContext) const
        {
            return GetLLVMType(Type, TmpContext);
        }

        [[nodiscard]] llvm::Type* GetLLVMType() const;
        [[nodiscard]] TypeCategory GetTypeCategory() const;
        [[nodiscard]] bool IsEqual(const DataType Other) const { return IsEqual(Type, Other.Type); }

        [[nodiscard]] operator DataTypeBase*() const { return Type; }
        [[nodiscard]] operator llvm::Type*() const { return GetLLVMType(); }
        [[nodiscard]] operator bool() const { return Type != nullptr; }
        [[nodiscard]] DataTypeBase* operator->() const { return Type; }
        [[nodiscard]] bool operator==(const DataType& Other) const { return Type == Other.Type; }
        [[nodiscard]] bool operator!=(const DataType& Other) const { return Type != Other.Type; }
        [[nodiscard]] bool operator==(std::nullptr_t) const { return Type == nullptr; }
        [[nodiscard]] bool operator!=(std::nullopt_t) const { return Type != nullptr; }

        [[nodiscard]] int GetPrimitiveTypeRank() const {return GetPrimitiveTypeRank(GetPrimitiveType()); }
        [[nodiscard]] int GetTypeRank(Arena& TypesArena) const { return GetTypeRank(Type, TypesArena); }

        friend class LLVMContextScope;
    };

    class LLVMContextScope
    {
    public:
        LLVMContextScope(llvm::LLVMContext& Context)
        {
            DataType::Context = &Context;
        }

        ~LLVMContextScope()
        {
            DataType::Context = nullptr;
            DataType::CachedContext = nullptr;
        }
    };
}

#endif //CVOLT_DATA_TYPE_H