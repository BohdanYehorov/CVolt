//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_DATATYPEBASE_H
#define CVOLT_DATATYPEBASE_H

#include <llvm/IR/DerivedTypes.h>

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Support/ErrorHandling.h"
#include <llvm/IR/Type.h>
#include <llvm/ADT/FoldingSet.h>

namespace Volt
{
    enum class TypeCategory : UInt8
    {
        Invalid,
        Void,
        Char,
        Boolean,
        Integer,
        FloatingPoint,
        Pointer,
        Reference,
        Array,
        Class,
        NullPointer,
        Function
    };

    enum class CastKind : UInt8
    {
        Exact,
        Ext,
        Trunc,
        CategoryConv,

        Explicit = 254,
        Invalid = 255
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
        virtual std::string GetIRName() const = 0;

        bool CastTo(DataType* To, bool Explicit) const
        {
            CastKind Rank = CastTo(To);
            if (Rank == CastKind::Invalid) return false;
            if (Rank == CastKind::Explicit) return Explicit;
            return true;
        }
        virtual CastKind CastTo(DataType* To) const = 0;

        [[nodiscard]] bool ImplicitCast(DataType* To) const { return CastTo(To, false); }
        [[nodiscard]] bool ExplicitCast(DataType* To) const { return CastTo(To, true); }

        [[nodiscard]] bool IsVoidType() const { return Category == TypeCategory::Void; }
        [[nodiscard]] bool IsBoolType() const { return Category == TypeCategory::Boolean; }
        [[nodiscard]] bool IsCharType() const { return Category == TypeCategory::Char; }
        [[nodiscard]] bool IsIntegerType() const { return Category == TypeCategory::Integer; }
        [[nodiscard]] bool IsFloatingPointType() const { return Category == TypeCategory::FloatingPoint; }
        [[nodiscard]] bool IsPointerType() const { return Category == TypeCategory::Pointer; }
        [[nodiscard]] bool IsNullPointerType() const { return Category == TypeCategory::NullPointer; }
        [[nodiscard]] bool IsReferenceType() const { return Category == TypeCategory::Reference; }
        [[nodiscard]] bool IsArrayType() const { return Category == TypeCategory::Array; }
        [[nodiscard]] bool IsClassType() const { return Category == TypeCategory::Class; }

        [[nodiscard]] bool IsSignedIntegerType() const;
        [[nodiscard]] bool IsUnsignedIntegerType() const
        {
            return Category == TypeCategory::Integer ? !IsSignedIntegerType() : false;
        }

        [[nodiscard]] bool IsAggregateType() const
        {
            return Category == TypeCategory::Array || Category == TypeCategory::Class;
        }

        [[nodiscard]] TypeCategory GetCategory() const { return Category; }

        template <typename ...Args_>
        [[nodiscard]] bool IsOneOf(Args_... Args) const
        {
            static_assert((std::same_as<Args_, TypeCategory> && ...));
            return ((Args == Category) || ...);
        }

        [[nodiscard]] llvm::Type* GetLLVMOrCachedType(llvm::LLVMContext& Context);

    protected:
        static DataType* GetJointType(DataType* Left, DataType* Right);

        friend class DataTypeHash;
        friend class CompilationContext;
        template<typename T>
        friend class Hash;

    public:
        static bool IsImplicitCastKind(CastKind Kind)
        {
            return Kind != CastKind::Invalid && Kind != CastKind::Explicit;
        }
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

        [[nodiscard]] CastKind CastTo(QualType To) const;

        [[nodiscard]] QualType GetNotReferenceType() const;

        [[nodiscard]] std::string ToString() const;
        [[nodiscard]] std::string GetIRName() const;
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
        VoidType() : PrimitiveDataType(TypeCategory::Void) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::Type::getVoidTy(Context);
        }

        int GetRank() const override { return 0; }
        std::string ToString() const override { return "void"; }
        size_t GetSize() const override { VoltUnreachable("Void type has not size"); }
        size_t GetAlignment() const override { VoltUnreachable("Void type has not alignment"); }
        std::string GetIRName() const override { return "v"; }

        CastKind CastTo(DataType *To) const override { return CastKind::Invalid; }
    };

    class BoolType : public PrimitiveDataType
    {
        GENERATED_BODY(BoolType, PrimitiveDataType)

    public:
        BoolType() : PrimitiveDataType(TypeCategory::Boolean) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getInt1Ty(Context);
        }

        int GetRank() const override { return 1; }
        std::string ToString() const override { return "bool"; }
        size_t GetSize() const override { return 1; }
        size_t GetAlignment() const override { return 1; }
        std::string GetIRName() const override { return "b"; }

        CastKind CastTo(DataType *To) const override;
    };

    class CharType : public PrimitiveDataType
    {
        GENERATED_BODY(CharType, PrimitiveDataType)

    public:
        CharType() : PrimitiveDataType(TypeCategory::Char) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getInt8Ty(Context);
        }

        int GetRank() const override { return 2; }
        std::string ToString() const override { return "char"; }
        size_t GetSize() const override { return 1; }
        size_t GetAlignment() const override { return 1; }
        std::string GetIRName() const override { return "c"; }

        CastKind CastTo(DataType *To) const override;
    };

    class IntegerType : public PrimitiveDataType
    {
        GENERATED_BODY(IntegerType, PrimitiveDataType)
    private:
        size_t BitWidth;
        bool bIsSigned;

    public:
        IntegerType(size_t BitWidth, bool IsSigned = true)
            : PrimitiveDataType(TypeCategory::Integer), BitWidth(BitWidth), bIsSigned(IsSigned) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::IntegerType::getIntNTy(Context, BitWidth);
        }

        int GetRank() const override { return std::countr_zero(BitWidth); }
        std::string ToString() const override;
        size_t GetSize() const override { return BitWidth/8; }
        size_t GetAlignment() const override { return BitWidth/8; }
        std::string GetIRName() const override;

        CastKind CastTo(DataType *To) const override;

        [[nodiscard]] size_t GetBitWidth() const { return BitWidth; }
        [[nodiscard]] size_t IsSigned() const { return bIsSigned; }
    };

    class FloatingPointType : public PrimitiveDataType
    {
        GENERATED_BODY(FloatingPointType, PrimitiveDataType)
    private:
        size_t BitWidth;

    public:
        FloatingPointType(size_t BitWidth)
            : PrimitiveDataType(TypeCategory::FloatingPoint), BitWidth(BitWidth) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override;
        int GetRank() const override { return std::countr_zero(BitWidth) + 3; }
        std::string ToString() const override;
        size_t GetSize() const override { return BitWidth/8; }
        size_t GetAlignment() const override { return BitWidth/8; }
        std::string GetIRName() const override;

        CastKind CastTo(DataType *To) const override;

        [[nodiscard]] size_t GetBitWidth() const { return BitWidth; }
    };

    class PointerType : public DataType, public llvm::FoldingSetNode
    {
        GENERATED_BODY(PointerType, DataType)
    private:
        QualType BaseType;

    public:
        PointerType(QualType BaseType)
            : DataType(TypeCategory::Pointer), BaseType(BaseType) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return 11; }
        std::string ToString() const override;
        size_t GetSize() const override { return 8; }
        size_t GetAlignment() const override { return 8; }
        std::string GetIRName() const override { return "P" + BaseType.GetIRName(); }

        void Profile(llvm::FoldingSetNodeID& ID) const
        {
            Profile(ID, BaseType);
        }

        static void Profile(llvm::FoldingSetNodeID& ID, QualType Pointee)
        {
            ID.AddPointer(Pointee.GetType());
            ID.AddInteger(Pointee.GetQuals());
        }

        CastKind CastTo(DataType *To) const override;

        [[nodiscard]] QualType GetBaseType() const { return BaseType; }
    };

    class NullPointerType : public DataType
    {
        GENERATED_BODY(NullPointerType, DataType)
    public:
        NullPointerType() : DataType(TypeCategory::NullPointer) {}
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return -1; }
        std::string ToString() const override { return "null_ty"; }
        size_t GetSize() const override { return 8; }
        size_t GetAlignment() const override { return 8; }
        std::string GetIRName() const override { return "n"; }

        CastKind CastTo(DataType *To) const override
        {
            return this == To || To->IsPointerType() ? CastKind::Exact : CastKind::Invalid;
        }
    };

    class ReferenceType : public DataType, public llvm::FoldingSetNode
    {
        GENERATED_BODY(ReferenceType, DataType)
    private:
        QualType BaseType;

    public:
        ReferenceType(QualType BaseType)
            : DataType(TypeCategory::Reference), BaseType(BaseType) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            return llvm::PointerType::get(Context, 0);
        }

        int GetRank() const override { return BaseType ? BaseType->GetRank() : -1; }
        std::string ToString() const override { return BaseType ? BaseType.ToString() + "$" : "?"; }
        size_t GetSize() const override { return BaseType->GetSize(); }
        size_t GetAlignment() const override { return BaseType->GetAlignment(); }
        std::string GetIRName() const override { return "R" + BaseType.GetIRName(); }

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

        CastKind CastTo(DataType *To) const override { return BaseType->CastTo(To); }

        [[nodiscard]] QualType GetBaseType() const { return BaseType; }
    };

    class ArrayType : public DataType, public llvm::FoldingSetNode
    {
        GENERATED_BODY(ArrayType, DataType)
    private:
        QualType BaseType;
        size_t Length;
        bool LengthInit;

    public:
        ArrayType(QualType BaseType, size_t Length) : DataType(TypeCategory::Array),
            BaseType(BaseType), Length(Length), LengthInit(true) {}
        ArrayType(QualType BaseType) : DataType(TypeCategory::Array),
            BaseType(BaseType), Length(0), LengthInit(false) {}

    public:
        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            if (!BaseType) return nullptr;
            return llvm::ArrayType::get(BaseType->GetLLVMOrCachedType(Context), Length);
        }

        int GetRank() const override { return 12; }
        std::string ToString() const override;
        size_t GetSize() const override
        {
            VoltAssert(LengthInit);
            return BaseType->GetSize() * Length;
        }
        size_t GetAlignment() const override { return BaseType->GetAlignment(); }
        std::string GetIRName() const override
        {
            return LengthInit ? "A" + std::to_string(Length) +
                BaseType->GetIRName() : "A" + BaseType->GetIRName();
        }

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

        CastKind CastTo(DataType *To) const override;

        [[nodiscard]] QualType GetBaseType() const { return BaseType; }
        [[nodiscard]] size_t GetLength() const { return Length; }
        [[nodiscard]] bool IsLengthInit() const { return LengthInit; }
    };

    class FunctionType : public DataType, public llvm::FoldingSetNode
    {
        GENERATED_BODY(FunctionType, DataType)
    protected:
        QualType ReturnType;
        ArgsVector<QualType> Params;

    public:
        FunctionType(QualType ReturnType, llvm::ArrayRef<QualType> Params)
            : DataType(TypeCategory::Function), ReturnType(ReturnType), Params(Params) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            ArgsVector<llvm::Type*> LLVMParams;
            LLVMParams.reserve(Params.size());
            for (QualType Param : Params)
                LLVMParams.push_back(Param->GetLLVMOrCachedType(Context));
            return llvm::FunctionType::get(ReturnType->GetLLVMOrCachedType(Context),
                LLVMParams, false);
        }

        int GetRank() const override { return -1; }
        std::string ToString() const override;
        std::string GetIRName() const override;

        size_t GetSize() const override { VoltUnreachable("Cannot get size from FunctionType"); }
        size_t GetAlignment() const override { VoltUnreachable("Cannot get alignment from FunctionType"); }

        CastKind CastTo(DataType *To) const override { return CastKind::Invalid; }

        [[nodiscard]] QualType GetReturnType() const { return ReturnType; }
        [[nodiscard]] llvm::ArrayRef<QualType> GetParams() const { return Params; }

        void Profile(llvm::FoldingSetNodeID& ID) const
        {
            Profile(ID, ReturnType, Params);
        }

        static void Profile(llvm::FoldingSetNodeID& ID,
            QualType ReturnType, llvm::ArrayRef<QualType> Params)
        {
            ID.AddInteger(ReturnType.RawValue());
            for (QualType Param : Params)
                ID.AddInteger(Param.RawValue());
        }
    };

    class MethodType : public FunctionType
    {
        GENERATED_BODY(MethodType, FunctionType)
    private:
        PointerType* ThisType;

    public:
        MethodType(QualType ReturnType, llvm::ArrayRef<QualType> Params, PointerType* ThisType)
            : FunctionType(ReturnType, Params), ThisType(ThisType) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override
        {
            ArgsVector<llvm::Type*> LLVMParams;
            LLVMParams.reserve(Params.size() + 1);
            LLVMParams.push_back(ThisType->GetLLVMOrCachedType(Context));
            for (QualType Param : Params)
                LLVMParams.push_back(Param->GetLLVMOrCachedType(Context));
            return llvm::FunctionType::get(ReturnType->GetLLVMOrCachedType(Context),
                LLVMParams, false);
        }

        std::string ToString() const override;
        std::string GetIRName() const override;

        [[nodiscard]] PointerType* GetThisType() const { return ThisType; }

        void Profile(llvm::FoldingSetNodeID& ID) const
        {
            Profile(ID, ReturnType, Params, ThisType);
        }

        static void Profile(llvm::FoldingSetNodeID& ID,
            QualType ReturnType, llvm::ArrayRef<QualType> Params, PointerType* ThisType)
        {
            ID.AddInteger(ReturnType.RawValue());
            for (QualType Param : Params)
                ID.AddInteger(Param.RawValue());
            ID.AddPointer(ThisType);
        }
    };
}

#endif //CVOLT_DATATYPEBASE_H