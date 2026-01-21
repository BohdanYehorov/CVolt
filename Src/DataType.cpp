//
// Created by bohdan on 08.01.26.
//

#include "Volt/Compiler/Types/DataType.h"
#include "Volt/Compiler/Types/CompilerTypes.h"
#include <complex>
#include <llvm/IR/DerivedTypes.h>

namespace Volt
{
    std::unordered_map<DataType::DataTypeNodeWrap, DataType, DataTypeHash> DataType::CachedTypes;
    DataTypeBase* DataType::CachedVoidType = nullptr;
    DataTypeBase* DataType::CachedBoolType = nullptr;
    DataTypeBase* DataType::CachedCharType = nullptr;
    DataTypeBase* DataType::CachedIntegerTypes[] = { nullptr, nullptr, nullptr, nullptr };
    DataTypeBase* DataType::CachedFPTypes[] = { nullptr, nullptr, nullptr, nullptr };

    llvm::LLVMContext* DataType::CachedContext = nullptr;
    llvm::LLVMContext* DataType::Context = nullptr;

    DataTypeBase* DataType::CreateVoid(Arena &TypesArena)
    {
        if (CachedVoidType)
            return CachedVoidType;

        CachedVoidType = TypesArena.Create<VoidType>();
        return CachedVoidType;
    }

    DataTypeBase* DataType::CreateBoolean(Arena &TypesArena)
    {
        if (CachedBoolType)
            return CachedBoolType;

        CachedBoolType = TypesArena.Create<BoolType>();
        return CachedBoolType;
    }

    DataTypeBase* DataType::CreateChar(Arena &TypesArena)
    {
        if (CachedCharType)
            return CachedCharType;

        CachedCharType = TypesArena.Create<CharType>();
        return CachedCharType;
    }

    DataTypeBase* DataType::CreateInteger(size_t BitWidth, Arena &TypesArena)
    {
        static size_t MinBitWidth = 8;

        assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 64);

        auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
        if (Index >= std::size(CachedIntegerTypes))
            throw std::range_error("Out of range");

        if (CachedIntegerTypes[Index])
            return CachedIntegerTypes[Index];

        CachedIntegerTypes[Index] = TypesArena.Create<IntegerType>(BitWidth);
        return CachedIntegerTypes[Index];
    }

    DataTypeBase* DataType::CreateFloatingPoint(size_t BitWidth, Arena &TypesArena)
    {
        static size_t MinBitWidth = 16;
        assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 128);

        auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
        if (Index >= std::size(CachedFPTypes))
            throw std::range_error("Out of range");

        if (CachedFPTypes[Index])
            return CachedFPTypes[Index];

        CachedFPTypes[Index] = TypesArena.Create<FloatingPointType>(BitWidth);
        return CachedFPTypes[Index];
    }

    DataTypeBase* DataType::CreatePtr(DataTypeBase *BaseType, Arena &TypesArena)
    {
        PointerType PtrDataType(BaseType);

        if (auto Iter = CachedTypes.find(&PtrDataType); Iter != CachedTypes.end())
        {
            std::cout << "Cached Ptr\n";
            return Iter->second;
        }

        auto PtrTypeNode = TypesArena.Create<PointerType>(BaseType);
        auto PtrType = PtrTypeNode;

        CachedTypes[PtrTypeNode] = PtrType;

        return PtrType;
    }

    llvm::Type* DataType::GetLLVMType(const DataTypeBase* Type, llvm::LLVMContext &Context)
    {
        if (Cast<const VoidType>(Type))
            return llvm::Type::getVoidTy(Context);
        if (Cast<const BoolType>(Type))
            return llvm::Type::getInt1Ty(Context);
        if (Cast<const CharType>(Type))
            return llvm::Type::getInt8Ty(Context);
        if (const auto Integer = Cast<const IntegerType>(Type))
            return llvm::Type::getIntNTy(Context, Integer->BitWidth);
        if (const auto Float = Cast<const FloatingPointType>(Type))
        {
            switch (Float->BitWidth) {
                case 16: return llvm::Type::getHalfTy(Context);
                case 32: return llvm::Type::getFloatTy(Context);
                case 64: return llvm::Type::getDoubleTy(Context);
                case 128: return llvm::Type::getFP128Ty(Context);
                default: throw std::runtime_error("Unsupported FP size");
            }
        }
        if (const auto PtrType = Cast<const PointerType>(Type))
            return llvm::PointerType::get(
                GetLLVMType(PtrType->BaseType, Context)->getContext(), 0);
        if (const auto RefType = Cast<const ReferenceType>(Type))
            return llvm::PointerType::get(
                GetLLVMType(RefType->BaseType, Context)->getContext(), 0);

        return nullptr;
    }

    bool DataType::IsEqual(const DataTypeBase *Left, const DataTypeBase *Right)
    {
        if (Left == Right) return true;
        if (!Left || !Right) return false;

        if (Cast<const VoidType>(Left) && Cast<const VoidType>(Right))
            return true;

        if (Cast<const BoolType>(Left) && Cast<const BoolType>(Right))
            return true;

        if (Cast<const CharType>(Left) && Cast<const CharType>(Right))
            return true;

        if (const auto LeftIntType = Cast<const IntegerType>(Left))
            if (const auto RightIntType = Cast<const IntegerType>(Right))
                return LeftIntType->BitWidth == RightIntType->BitWidth;

        if (const auto LeftFloatType = Cast<const FloatingPointType>(Left))
            if (const auto RightFloatType = Cast<const FloatingPointType>(Right))
                return LeftFloatType->BitWidth == RightFloatType->BitWidth;

        if (const auto LeftPtrType = Cast<const PointerType>(Left))
            if (const auto RightPtrType = Cast<const PointerType>(Right))
                return IsEqual(LeftPtrType->BaseType, RightPtrType->BaseType);

        if (const auto LeftRefType = Cast<const ReferenceType>(Left))
            if (const auto RightRefType = Cast<const ReferenceType>(Right))
                return IsEqual(LeftRefType->BaseType, RightRefType->BaseType);

        return false;
    }

    int DataType::GetPrimitiveTypeRank(const PrimitiveDataType *Type)
    {
        if (Cast<const VoidType>(Type))
            return 0;

        if (Cast<const BoolType>(Type))
            return 1;

        if (Cast<const CharNode>(Type))
            return 2;

        if (const auto IntType = Cast<const IntegerType>(Type))
        {
            switch (IntType->BitWidth)
            {
                case 8:  return  3;
                case 16: return  4;
                case 32: return  5;
                case 64: return  6;
                default: return -1;
            }
        }

        if (const auto FloatType = Cast<const FloatingPointType>(Type))
        {
            switch (FloatType->BitWidth)
            {
                case 16:  return  7;
                case 32:  return  8;
                case 64:  return  9;
                case 128: return 10;
                default:  return -1;
            }
        }

        return -1;
    }

    int DataType::GetTypeRank(const DataTypeBase *Type, Arena &TypesArena)
    {
        if (const auto PrimitiveType =  Cast<const PrimitiveDataType>(Type))
            return GetPrimitiveTypeRank(PrimitiveType);

        static int MaxPrimitiveTypeRank = GetPrimitiveTypeRank(
            Cast<PrimitiveDataType>(CreateFloatingPoint(128, TypesArena)));

        if (Cast<const PointerType>(Type))
            return MaxPrimitiveTypeRank + 1;

        return -1;
    }

    int DataType::GetTypeBitWidth() const
    {
        if (Cast<VoidType>(Type))
            return 0;

        if (Cast<BoolType>(Type))
            return 1;

        if (Cast<CharNode>(Type))
            return 8;

        if (const auto IntType = Cast<IntegerType>(Type))
            return static_cast<int>(IntType->BitWidth);

        if (const auto FloatType = Cast<FloatingPointType>(Type))
            return static_cast<int>(FloatType->BitWidth);

        return -1;
    }

    llvm::Type * DataType::GetLLVMType() const
    {
        if (!Context || !Type)
            return nullptr;

        if (CachedContext != Context)
        {
            CachedContext = Context;
            Type->CachedType = GetLLVMType(*Context);
        }
        else if (!Type->CachedType)
            Type->CachedType = GetLLVMType(*Context);

        return Type->CachedType;
    }

    TypeCategory DataType::GetTypeCategory() const
    {
        if (Cast<VoidType>(Type))
            return TypeCategory::VOID;
        if (Cast<BoolType>(Type))
            return TypeCategory::BOOLEAN;
        if (Cast<IntegerType>(Type))
            return TypeCategory::INTEGER;
        if (Cast<FloatingPointType>(Type))
            return TypeCategory::FLOATING_POINT;
        if (Cast<PointerType>(Type))
            return TypeCategory::POINTER;
        if (Cast<ReferenceType>(Type))
            return TypeCategory::REFERENCE;

        return TypeCategory::INVALID;
    }
}
