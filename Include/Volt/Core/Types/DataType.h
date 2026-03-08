//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_DATATYPEBASE_H
#define CVOLT_DATATYPEBASE_H

#include <llvm/IR/DerivedTypes.h>

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include "Volt/Core/Enums/OperatorType.h"
#include <llvm/IR/Type.h>

namespace Volt
{
    enum class TypeCategory : UInt8
    {
        INVALID,
        VOID,
        CHAR,
        BOOLEAN,
        INTEGER,
        FLOATING_POINT,
        POINTER,
        REFERENCE,
        ARRAY,
        CONSTANT
    };

    class DataType : public Object
    {
        GENERATED_BODY(DataTypeBase, Object)
    private:
        mutable size_t CachedHash = 0;
        llvm::Type* CachedType = nullptr;

    public:
        virtual bool IsEqual(const DataType* Other) const = 0;
        virtual llvm::Type* ToLLVMType(llvm::LLVMContext& Context) const = 0;
        virtual int GetRank() const = 0;
        virtual std::string ToString() const = 0;
        virtual TypeCategory GetCategory() const = 0;

        virtual DataType* ImplicitCast(DataType* To) const = 0;

    protected:
        static DataType* GetJointType(DataType* Left, DataType* Right);

        friend class DataTypeHash;
        friend class CompilationContext;
    };

    class PrimitiveDataType : public DataType
    {
        GENERATED_BODY(PrimitiveDataType, DataType)
    };

    class VoidType : public PrimitiveDataType
    {
        GENERATED_BODY(VoidType, PrimitiveDataType)

    public:
        bool IsEqual(const DataType* Other) const override
        {
            return Cast<const VoidType>(Other) != nullptr;
        }

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::Type::getVoidTy(Context);
        }

        int GetRank() const override { return 0; }
        std::string ToString() const override { return "void"; }
        TypeCategory GetCategory() const override { return TypeCategory::VOID; }
        DataType* ImplicitCast(DataType *To) const override { return nullptr; }
    };

    class BoolType : public PrimitiveDataType
    {
        GENERATED_BODY(BoolType, PrimitiveDataType)

    public:
        bool IsEqual(const DataType* Other) const override
        {
            return Cast<const BoolType>(Other) != nullptr;
        }

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getInt1Ty(Context);
        }

        int GetRank() const override { return 1; }
        std::string ToString() const override { return "bool"; }
        TypeCategory GetCategory() const override { return TypeCategory::BOOLEAN; }
        DataType* ImplicitCast(DataType *To) const override;
    };

    class CharType : public PrimitiveDataType
    {
        GENERATED_BODY(CharType, PrimitiveDataType)

    public:
        bool IsEqual(const DataType* Other) const override
        {
            return Cast<const CharType>(Other) != nullptr;
        }

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getInt8Ty(Context);
        }

        int GetRank() const override { return 1; }
        std::string ToString() const override { return "char"; }
        TypeCategory GetCategory() const override { return TypeCategory::CHAR; }
        DataType* ImplicitCast(DataType *To) const override;
    };

    class IntegerType : public PrimitiveDataType
    {
        GENERATED_BODY(IntegerType, PrimitiveDataType)
    public:
        size_t BitWidth;
        bool IsSigned;
        IntegerType(size_t BitWidth, bool IsSigned = false)
            : BitWidth(BitWidth), IsSigned(IsSigned) {}

    public:
        bool IsEqual(const DataType* Other) const override;

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getIntNTy(Context, BitWidth);
        }

        int GetRank() const override;
        std::string ToString() const override;
        TypeCategory GetCategory() const override { return TypeCategory::INTEGER; }
        DataType* ImplicitCast(DataType *To) const override;
    };

    class FloatingPointType : public PrimitiveDataType
    {
        GENERATED_BODY(FloatingPointType, PrimitiveDataType)
    public:
        size_t BitWidth;
        FloatingPointType(size_t BitWidth) : BitWidth(BitWidth) {}

    public:
        bool IsEqual(const DataType* Other) const override;
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override;
        int GetRank() const override;
        std::string ToString() const override;
        TypeCategory GetCategory() const override { return TypeCategory::FLOATING_POINT; }
        DataType* ImplicitCast(DataType *To) const override;
    };

    class PointerType : public DataType
    {
        GENERATED_BODY(PointerType, DataType)
    public:
        DataType* BaseType;
        PointerType(DataType* BaseType)
            : BaseType(BaseType) {}

    public:
        bool IsEqual(const DataType* Other) const override;

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return 11; }
        std::string ToString() const override { return BaseType ? BaseType->ToString() + "*" : "?"; }
        TypeCategory GetCategory() const override { return TypeCategory::POINTER; }
        DataType* ImplicitCast(DataType *To) const override;
    };

    class ReferenceType : public DataType
    {
        GENERATED_BODY(ReferenceType, DataType)
    public:
        DataType* BaseType;
        ReferenceType(DataType* BaseType)
            : BaseType(BaseType) {}

    public:
        bool IsEqual(const DataType* Other) const override;

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return BaseType ? BaseType->GetRank() : -1; }
        std::string ToString() const override { return BaseType ? BaseType->ToString() + "$" : "?"; }
        TypeCategory GetCategory() const override { return TypeCategory::REFERENCE; }
        DataType* ImplicitCast(DataType *To) const override
        {
            throw std::runtime_error("Cast references is unsupported");
        }
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

    public:
        bool IsEqual(const DataType* Other) const override;

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            if (!BaseType) return nullptr;
            return llvm::ArrayType::get(BaseType->ToLLVMType(Context), Length);
        }

        int GetRank() const override { return 12; }
        std::string ToString() const override;
        TypeCategory GetCategory() const override { return TypeCategory::ARRAY; }
        DataType* ImplicitCast(DataType *To) const override;
    };

    class ConstType : public DataType
    {
        GENERATED_BODY(ConstType, DataType)
    public:
        DataType* BaseType;
        ConstType(DataType* BaseType)
            : BaseType(BaseType) {}

    public:
        bool IsEqual(const DataType* Other) const override;

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            if (!BaseType) return nullptr;
            return BaseType->ToLLVMType(Context);
        }

        int GetRank() const override { return BaseType ? BaseType->GetRank() : -1; }
        std::string ToString() const override { return BaseType ? "const " + BaseType->ToString() : "?"; }
        TypeCategory GetCategory() const override { return TypeCategory::CONSTANT; }
        DataType* ImplicitCast(DataType *To) const override;
    };
}

#endif //CVOLT_DATATYPEBASE_H