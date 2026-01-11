//
// Created by bohdan on 08.01.26.
//

#include "DataType.h"

#include <complex>
#include <llvm/IR/DerivedTypes.h>
#include "CompilerTypes.h"

namespace Volt
{
    std::unordered_map<DataType::DataTypeNodeWrap, DataType*, DataTypeHash> DataType::CachedTypes;
    DataType* DataType::CachedVoidType = nullptr;
    DataType* DataType::CachedBoolType = nullptr;
    DataType* DataType::CachedCharType = nullptr;
    DataType* DataType::CachedIntegerTypes[] = { nullptr, nullptr, nullptr, nullptr };
    DataType* DataType::CachedFPTypes[] = { nullptr, nullptr, nullptr, nullptr };

    DataType *DataType::Create(DataTypeNodeBase *Base, Arena &TypesArena)
    {
        if (auto Iter = CachedTypes.find(Base); Iter != CachedTypes.end())
        {
            std::cout << "Cached Type\n";
            return Iter->second;
        }

        DataType* Type = TypesArena.Create<DataType>(Base);
        CachedTypes[Base] = Type;
        return Type;
    }

    DataType *DataType::CreateVoid(Arena &TypesArena)
    {
        if (CachedVoidType)
            return CachedVoidType;

        CachedVoidType = TypesArena.Create<DataType>(TypesArena.Create<VoidTypeNode>());
        return CachedVoidType;
    }

    DataType *DataType::CreateBoolean(Arena &TypesArena)
    {
        if (CachedBoolType)
            return CachedBoolType;

        CachedBoolType = TypesArena.Create<DataType>(TypesArena.Create<BoolTypeNode>());
        return CachedBoolType;
    }

    DataType *DataType::CreateChar(Arena &TypesArena)
    {
        if (CachedCharType)
            return CachedCharType;

        CachedCharType = TypesArena.Create<DataType>(TypesArena.Create<CharTypeNode>());
        return CachedCharType;
    }

    DataType *DataType::CreateInteger(size_t BitWidth, Arena &TypesArena)
    {
        static size_t MinBitWidth = 8;

        assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 64);

        auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
        if (Index >= std::size(CachedIntegerTypes))
            throw std::range_error("Out of range");

        if (CachedIntegerTypes[Index])
            return CachedIntegerTypes[Index];

        CachedIntegerTypes[Index] = TypesArena.Create<DataType>(TypesArena.Create<IntegerTypeNode>(BitWidth));
        return CachedIntegerTypes[Index];
    }

    DataType *DataType::CreateFloatingPoint(size_t BitWidth, Arena &TypesArena)
    {
        static size_t MinBitWidth = 16;
        assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 128);

        auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
        if (Index >= std::size(CachedFPTypes))
            throw std::range_error("Out of range");

        if (CachedFPTypes[Index])
            return CachedFPTypes[Index];

        CachedFPTypes[Index] = TypesArena.Create<DataType>(TypesArena.Create<FPTypeNode>(BitWidth));
        return CachedFPTypes[Index];
    }

    DataType* DataType::CreatePtr(DataTypeNodeBase *BaseType, Arena &TypesArena)
    {
        PtrDataTypeNode PtrDataType(BaseType);

        if (auto Iter = CachedTypes.find(&PtrDataType); Iter != CachedTypes.end())
        {
            std::cout << "Cached Ptr\n";
            return Iter->second;
        }

        auto PtrTypeNode = TypesArena.Create<PtrDataTypeNode>(BaseType);
        auto PtrType = TypesArena.Create<DataType>(PtrTypeNode);

        CachedTypes[PtrTypeNode] = PtrType;

        return PtrType;
    }

    llvm::Type* DataType::ToLLVMPrimitiveType(PrimitiveDataType Type, llvm::LLVMContext &Context)
    {
        switch (Type)
        {
            case PrimitiveDataType::VOID:
                return llvm::Type::getVoidTy(Context);
            case PrimitiveDataType::BOOL:
                return llvm::Type::getInt1Ty(Context);
            case PrimitiveDataType::CHAR:
            case PrimitiveDataType::BYTE:
                return llvm::Type::getInt8Ty(Context);
            case PrimitiveDataType::INT:
                return llvm::Type::getInt32Ty(Context);
            case PrimitiveDataType::LONG:
                return llvm::Type::getInt64Ty(Context);
            case PrimitiveDataType::FLOAT:
                return llvm::Type::getFloatTy(Context);
            case PrimitiveDataType::DOUBLE:
                return llvm::Type::getDoubleTy(Context);
            default:
                return nullptr;
        }
    }

    llvm::Type* DataType::GetLLVMType(const DataTypeNodeBase* Type, llvm::LLVMContext &Context)
    {
        if (Cast<const VoidTypeNode>(Type))
            return llvm::Type::getVoidTy(Context);
        if (Cast<const BoolTypeNode>(Type))
            return llvm::Type::getInt1Ty(Context);
        if (Cast<const CharTypeNode>(Type))
            return llvm::Type::getInt8Ty(Context);
        if (const auto Integer = Cast<const IntegerTypeNode>(Type))
            return llvm::Type::getIntNTy(Context, Integer->BitWidth);
        if (const auto Float = Cast<const FPTypeNode>(Type))
        {
            switch (Float->BitWidth) {
                case 16: return llvm::Type::getHalfTy(Context);
                case 32: return llvm::Type::getFloatTy(Context);
                case 64: return llvm::Type::getDoubleTy(Context);
                case 128: return llvm::Type::getFP128Ty(Context);
                default: throw std::runtime_error("Unsupported FP size");
            }
        }
        if (const auto PtrType = Cast<const PtrDataTypeNode>(Type))
            return llvm::PointerType::get(
                GetLLVMType(PtrType->BaseType, Context)->getContext(), 0);
        if (const auto RefType = Cast<const RefDataTypeNode>(Type))
            return llvm::PointerType::get(
                GetLLVMType(RefType->BaseType, Context)->getContext(), 0);

        return nullptr;
    }

    bool DataType::IsEqual(const DataTypeNodeBase *Left, const DataTypeNodeBase *Right)
    {
        if (Left == Right) return true;
        if (!Left || !Right) return false;

        if (Cast<const VoidTypeNode>(Left) && Cast<const VoidTypeNode>(Right))
            return true;

        if (Cast<const BoolTypeNode>(Left) && Cast<const BoolTypeNode>(Right))
            return true;

        if (Cast<const CharTypeNode>(Left) && Cast<const CharTypeNode>(Right))
            return true;

        if (const auto LeftIntType = Cast<const IntegerTypeNode>(Left))
            if (const auto RightIntType = Cast<const IntegerTypeNode>(Right))
                return LeftIntType->BitWidth == RightIntType->BitWidth;

        if (const auto LeftFloatType = Cast<const FPTypeNode>(Left))
            if (const auto RightFloatType = Cast<const FPTypeNode>(Right))
                return LeftFloatType->BitWidth == RightFloatType->BitWidth;

        if (const auto LeftPtrType = Cast<const PtrDataTypeNode>(Left))
            if (const auto RightPtrType = Cast<const PtrDataTypeNode>(Right))
                return IsEqual(LeftPtrType->BaseType, RightPtrType->BaseType);

        if (const auto LeftRefType = Cast<const RefDataTypeNode>(Left))
            if (const auto RightRefType = Cast<const RefDataTypeNode>(Right))
                return IsEqual(LeftRefType->BaseType, RightRefType->BaseType);

        return false;
    }

    int DataType::GetPrimitiveTypeRank(const PrimitiveDataTypeNode *Type)
    {
        if (Cast<const VoidTypeNode>(Type))
            return 0;

        if (Cast<const BoolTypeNode>(Type))
            return 1;

        if (Cast<const CharNode>(Type))
            return 2;

        if (const auto IntType = Cast<const IntegerTypeNode>(Type))
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

        if (const auto FloatType = Cast<const FPTypeNode>(Type))
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

    int DataType::GetTypeBitWidth() const
    {
        if (Cast<VoidTypeNode>(Type))
            return 0;

        if (Cast<BoolTypeNode>(Type))
            return 1;

        if (Cast<CharNode>(Type))
            return 8;

        if (const auto IntType = Cast<IntegerTypeNode>(Type))
            return static_cast<int>(IntType->BitWidth);

        if (const auto FloatType = Cast<FPTypeNode>(Type))
            return static_cast<int>(FloatType->BitWidth);

        return -1;
    }

    llvm::Type* DataType::GetLLVMType(llvm::LLVMContext& Context)
    {
        if (CachedContext != &Context)
        {
            CachedContext = &Context;
            CachedType = GetLLVMType(Type, Context);
            return CachedType;
        }

        if (CachedType)
            return CachedType;

        CachedType = GetLLVMType(Type, Context);
        return CachedType;
    }
}
