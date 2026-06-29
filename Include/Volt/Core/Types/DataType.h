//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_DATATYPEBASE_H
#define CVOLT_DATATYPEBASE_H

#include <llvm/IR/DerivedTypes.h>

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include "Volt/Support/ErrorHandling.h"
#include "Volt/ADT/Array.h"
#include <llvm/IR/Type.h>
#include <llvm/ADT/FoldingSet.h>
#include <llvm/ADT/TinyPtrVector.h>

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
        CLASS,
        NULL_POINTER
    };

    class alignas(8) DataType : public Object
    {
        GENERATED_BODY(DataType, Object)
    private:
        mutable size_t CachedHash = 0;
        llvm::Type* CachedType = nullptr;

    protected:
        TypeCategory Category;

    public:
        DataType(TypeCategory Category) : Category(Category) {}

        virtual llvm::Type* ToLLVMType(llvm::LLVMContext& Context) const = 0;
        virtual int GetRank() const = 0;
        virtual std::string ToString() const = 0;
        virtual size_t GetSize() const = 0;
        virtual size_t GetAlignment() const = 0;

        virtual bool CastTo(DataType* To, bool Explicit) const = 0;

        [[nodiscard]] bool ImplicitCast(DataType* To) const { return CastTo(To, false); }
        [[nodiscard]] bool ExplicitCast(DataType* To) const { return CastTo(To, true); }

        [[nodiscard]] bool IsVoidType() const { return Category == TypeCategory::VOID; }
        [[nodiscard]] bool IsBoolType() const { return Category == TypeCategory::BOOLEAN; }
        [[nodiscard]] bool IsCharType() const { return Category == TypeCategory::CHAR; }
        [[nodiscard]] bool IsIntegerType() const { return Category == TypeCategory::INTEGER; }
        [[nodiscard]] bool IsFloatingPointType() const { return Category == TypeCategory::FLOATING_POINT; }
        [[nodiscard]] bool IsPointerType() const { return Category == TypeCategory::POINTER; }
        [[nodiscard]] bool IsNullPointerType() const { return Category == TypeCategory::NULL_POINTER; }
        [[nodiscard]] bool IsReferenceType() const { return Category == TypeCategory::REFERENCE; }
        [[nodiscard]] bool IsArrayType() const { return Category == TypeCategory::ARRAY; }
        [[nodiscard]] bool IsClassType() const { return Category == TypeCategory::CLASS; }

        [[nodiscard]] bool IsSignedIntegerType() const;
        [[nodiscard]] bool IsUnsignedIntegerType() const
        {
            return Category == TypeCategory::INTEGER ? !IsSignedIntegerType() : false;
        }

        [[nodiscard]] TypeCategory GetCategory() const { return Category; }

        template <typename ...Args_>
        [[nodiscard]] bool IsOneOf(Args_... Args) const
        {
            static_assert((std::same_as<Args_, TypeCategory> && ...));
            return ((Args == Category) || ...);
        }
    protected:
        static DataType* GetJointType(DataType* Left, DataType* Right);

    private:
        friend class DataTypeHash;
        friend class CompilationContext;
        template<typename T>
        friend class Hash;
    };

    class QualType
    {
        static_assert(alignof(DataType) >= 8);

    public:
        enum QualifierKind
        {
            CONST = 1 << 0
        };

    private:
        UIntPtrTy Value;

    public:
        QualType() : Value(0) {}

        QualType(DataType* Type)
        {
            Value = reinterpret_cast<UIntPtrTy>(Type);
        }

        QualType(DataType* Type, UInt32 Quals)
        {
            VoltAssert(Quals < alignof(DataType));
            Value = reinterpret_cast<UIntPtrTy>(Type) | Quals;
        }

        operator bool() const { return GetType() != nullptr; }
        DataType* operator->() const { return GetType(); }

        [[nodiscard]] bool operator==(const QualType& Other) const
        {
            return Value == Other.Value;
        }

        [[nodiscard]] bool operator!=(const QualType& Other) const
        {
            return Value != Other.Value;
        }

        [[nodiscard]] DataType* GetType() const
        {
            return reinterpret_cast<DataType*>(Value & ~(alignof(DataType) - 1));
        }

        [[nodiscard]] UIntPtrTy RawValue() const { return Value; }

        template <typename T>
        [[nodiscard]] T* CastAs() const
        {
            return Cast<T>(GetType());
        }

        [[nodiscard]] UInt32 GetQuals() const { return Value & (alignof(DataType) - 1); }
        [[nodiscard]] bool HasQualifier(QualifierKind Kind) const { return (Value & Kind) != 0; }

        void AddQualifiers(UInt32 Qualifiers)
        {
            VoltAssert(Qualifiers < alignof(DataType));
            Value |= Qualifiers;
        }

        void RemoveQualifiers(UInt32 Qualifiers)
        {
            VoltAssert(Qualifiers < alignof(DataType));
            Value &= ~Qualifiers;
        }

        [[nodiscard]] bool CastTo(QualType To, bool Explicit) const;
        [[nodiscard]] bool ImplicitCast(QualType To) const { return CastTo(To, false); }
        [[nodiscard]] bool ExplicitCast(QualType To) const { return CastTo(To, true); }

        [[nodiscard]] QualType GetNotReferenceType() const;

        [[nodiscard]] std::string ToString() const;
    };

    class PrimitiveDataType : public DataType
    {
        GENERATED_BODY(PrimitiveDataType, DataType)
    public:
        PrimitiveDataType(TypeCategory Category) : DataType(Category) {}
    };

    class VoidType : public PrimitiveDataType
    {
        GENERATED_BODY(VoidType, PrimitiveDataType)

    public:
        VoidType() : PrimitiveDataType(TypeCategory::VOID) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::Type::getVoidTy(Context);
        }

        int GetRank() const override { return 0; }
        std::string ToString() const override { return "void"; }
        size_t GetSize() const override { VoltUnreachable("Void type has not size"); }
        size_t GetAlignment() const override { VoltUnreachable("Void type has not alignment"); }

        bool CastTo(DataType *To, bool Explicit) const override { return false; }
    };

    class BoolType : public PrimitiveDataType
    {
        GENERATED_BODY(BoolType, PrimitiveDataType)

    public:
        BoolType() : PrimitiveDataType(TypeCategory::BOOLEAN) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getInt1Ty(Context);
        }

        int GetRank() const override { return 1; }
        std::string ToString() const override { return "bool"; }
        size_t GetSize() const override { return 1; }
        size_t GetAlignment() const override { return 1; }

        bool CastTo(DataType* To, bool Explicit) const override;
    };

    class CharType : public PrimitiveDataType
    {
        GENERATED_BODY(CharType, PrimitiveDataType)

    public:
        CharType() : PrimitiveDataType(TypeCategory::CHAR) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getInt8Ty(Context);
        }

        int GetRank() const override { return 2; }
        std::string ToString() const override { return "char"; }
        size_t GetSize() const override { return 1; }
        size_t GetAlignment() const override { return 1; }

        bool CastTo(DataType *To, bool Explicit) const override;
    };

    class IntegerType : public PrimitiveDataType
    {
        GENERATED_BODY(IntegerType, PrimitiveDataType)
    public:
        size_t BitWidth;
        bool IsSigned;
        IntegerType(size_t BitWidth, bool IsSigned = true)
            : PrimitiveDataType(TypeCategory::INTEGER), BitWidth(BitWidth), IsSigned(IsSigned) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getIntNTy(Context, BitWidth);
        }

        int GetRank() const override { return std::countr_zero(BitWidth); }
        std::string ToString() const override;
        size_t GetSize() const override { return BitWidth/8; }
        size_t GetAlignment() const override { return BitWidth/8; }

        bool CastTo(DataType *To, bool Explicit) const override;
    };

    class FloatingPointType : public PrimitiveDataType
    {
        GENERATED_BODY(FloatingPointType, PrimitiveDataType)
    public:
        size_t BitWidth;
        FloatingPointType(size_t BitWidth)
            : PrimitiveDataType(TypeCategory::FLOATING_POINT), BitWidth(BitWidth) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override;
        int GetRank() const override { return std::countr_zero(BitWidth) + 3; }
        std::string ToString() const override;
        size_t GetSize() const override { return BitWidth/8; }
        size_t GetAlignment() const override { return BitWidth/8; }

        bool CastTo(DataType *To, bool Explicit) const override;
    };

    class PointerType : public DataType, public llvm::FoldingSetNode
    {
        GENERATED_BODY(PointerType, DataType)
    public:
        QualType BaseType;
        PointerType(QualType BaseType)
            : DataType(TypeCategory::POINTER), BaseType(BaseType) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return 11; }
        std::string ToString() const override;
        size_t GetSize() const override { return 8; }
        size_t GetAlignment() const override { return 8; }

        void Profile(llvm::FoldingSetNodeID& ID) const
        {
            Profile(ID, BaseType);
        }

        static void Profile(llvm::FoldingSetNodeID& ID, QualType Pointee)
        {
            ID.AddPointer(Pointee.GetType());
            ID.AddInteger(Pointee.GetQuals());
        }

        bool CastTo(DataType *To, bool Explicit) const override;
    };

    class NullPointerType : public DataType
    {
        GENERATED_BODY(NullPointerType, DataType)
    public:
        NullPointerType() : DataType(TypeCategory::NULL_POINTER) {}
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return -1; }
        std::string ToString() const override { return "null_ty"; }
        size_t GetSize() const override { return 8; }
        size_t GetAlignment() const override { return 8; }

        bool CastTo(DataType *To, bool Explicit) const override { return this == To || To->IsPointerType(); }
    };

    class ReferenceType : public DataType, public llvm::FoldingSetNode
    {
        GENERATED_BODY(ReferenceType, DataType)
    public:
        QualType BaseType;
        ReferenceType(QualType BaseType)
            : DataType(TypeCategory::REFERENCE), BaseType(BaseType) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return BaseType ? BaseType->GetRank() : -1; }
        std::string ToString() const override { return BaseType ? BaseType.ToString() + "$" : "?"; }
        size_t GetSize() const override { return BaseType->GetSize(); }
        size_t GetAlignment() const override { return BaseType->GetAlignment(); }

        bool CanBind(QualType Type) const;

        void Profile(llvm::FoldingSetNodeID& ID) const
        {
            Profile(ID, BaseType);
        }

        static void Profile(llvm::FoldingSetNodeID& ID, QualType BaseType)
        {
            ID.AddPointer(BaseType.GetType());
            ID.AddInteger(BaseType.GetQuals());
        }

        bool CastTo(DataType *To, bool Explicit) const override;
    };

    class ArrayType : public DataType, public llvm::FoldingSetNode
    {
        GENERATED_BODY(ArrayType, DataType)
    public:
        QualType BaseType;
        size_t Length;
        bool LengthInit;

        ArrayType(QualType BaseType, size_t Length) : DataType(TypeCategory::ARRAY),
            BaseType(BaseType), Length(Length), LengthInit(true) {}
        ArrayType(QualType BaseType) : DataType(TypeCategory::ARRAY),
            BaseType(BaseType), Length(0), LengthInit(false) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            if (!BaseType) return nullptr;
            return llvm::ArrayType::get(BaseType->ToLLVMType(Context), Length);
        }

        int GetRank() const override { return 12; }
        std::string ToString() const override;
        size_t GetSize() const override
        {
            VoltAssert(LengthInit);
            return BaseType->GetSize() * Length;
        }
        size_t GetAlignment() const override { return BaseType->GetAlignment(); }

        void Profile(llvm::FoldingSetNodeID& ID) const
        {
            Profile(ID, BaseType, Length, LengthInit);
        }

        static void Profile(llvm::FoldingSetNodeID& ID, QualType BaseType, size_t Length, bool LengthInit)
        {
            ID.AddPointer(BaseType.GetType());
            ID.AddInteger(BaseType.GetQuals());
            ID.AddInteger(Length);
            ID.AddBoolean(LengthInit);
        }

        bool CastTo(DataType *To, bool Explicit) const override;
    };
}

#endif //CVOLT_DATATYPEBASE_H