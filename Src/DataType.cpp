//
// Created by bohdan on 08.01.26.
//

#include "Volt/Compiler/Types/DataType.h"
#include "Volt/Compiler/Types/CompilerTypes.h"
#include "Volt/Core/AST/ASTNodes.h"
#include <complex>
#include <unordered_set>
#include <llvm/IR/DerivedTypes.h>

namespace Volt
{
    std::unordered_set<DataType::DataTypeNodeWrap, DataTypeHash> DataType::CachedTypes;
    VoidType* DataType::CachedVoidType = nullptr;
    BoolType* DataType::CachedBoolType = nullptr;
    CharType* DataType::CachedCharType = nullptr;
    IntegerType* DataType::CachedIntegerTypes[] = { nullptr, nullptr, nullptr, nullptr };
    FloatingPointType* DataType::CachedFPTypes[] = { nullptr, nullptr, nullptr, nullptr };

    llvm::LLVMContext* DataType::CachedContext = nullptr;
    llvm::LLVMContext* DataType::Context = nullptr;

    VoidType* DataType::CreateVoid(Arena &TypesArena)
    {
        if (CachedVoidType)
        {
            //std::cout << "Void Type From Cache\n";
            return CachedVoidType;
        }

        CachedVoidType = TypesArena.Create<VoidType>();
        return CachedVoidType;
    }

    BoolType* DataType::CreateBoolean(Arena &TypesArena)
    {
        if (CachedBoolType)
        {
            //std::cout << "Bool Type From Cache\n";
            return CachedBoolType;
        }

        CachedBoolType = TypesArena.Create<BoolType>();
        return CachedBoolType;
    }

    CharType* DataType::CreateChar(Arena &TypesArena)
    {
        if (CachedCharType)
        {
            //std::cout << "Char Type From Cache\n";
            return CachedCharType;
        }

        CachedCharType = TypesArena.Create<CharType>();
        return CachedCharType;
    }

    IntegerType* DataType::CreateInteger(size_t BitWidth, Arena &TypesArena)
    {
        static size_t MinBitWidth = 8;

        assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 64);

        auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
        if (Index >= std::size(CachedIntegerTypes))
            throw std::range_error("Out of range");

        if (CachedIntegerTypes[Index])
        {
            //std::cout << "Int" << BitWidth << " Type From Cache\n";
            return CachedIntegerTypes[Index];
        }

        CachedIntegerTypes[Index] = TypesArena.Create<IntegerType>(BitWidth);
        return CachedIntegerTypes[Index];
    }

    FloatingPointType* DataType::CreateFloatingPoint(size_t BitWidth, Arena &TypesArena)
    {
        static size_t MinBitWidth = 16;
        assert(BitWidth % 8 == 0 && BitWidth >= MinBitWidth && BitWidth <= 128);

        auto Index = static_cast<size_t>(std::log2(BitWidth / MinBitWidth));
        if (Index >= std::size(CachedFPTypes))
            throw std::range_error("Out of range");

        if (CachedFPTypes[Index])
        {
            //std::cout << "Float" << BitWidth << " Type From Cache\n";
            return CachedFPTypes[Index];
        }

        CachedFPTypes[Index] = TypesArena.Create<FloatingPointType>(BitWidth);
        return CachedFPTypes[Index];
    }

    PointerType* DataType::CreatePtr(DataTypeBase *BaseType, Arena &TypesArena)
    {
        PointerType PtrDataType(BaseType);

        if (auto Iter = CachedTypes.find(&PtrDataType); Iter != CachedTypes.end())
        {
            std::cout << "Pointer From Cache\n";
            return Cast<PointerType>(Iter->Type);
        }

        auto PtrTypeNode = TypesArena.Create<PointerType>(BaseType);
        CachedTypes.insert(PtrTypeNode);

        return PtrTypeNode;
    }

    ArrayType* DataType::CreateArray(DataTypeBase *BaseType, size_t Length, Arena &TypesArena)
    {
        ArrayType ArrDataType(BaseType, Length);

        if (auto Iter = CachedTypes.find(&ArrDataType); Iter != CachedTypes.end())
        {
            std::cout << "Array From Cache\n";
            return Cast<ArrayType>(Iter->Type);
        }

        auto ArrType = TypesArena.Create<ArrayType>(BaseType, Length);
        CachedTypes.insert(ArrType);

        return ArrType;
    }

    DataTypeBase* DataType::CreateFromAST(const DataTypeNodeBase *Node, Arena &TypesArena)
    {
        if (const auto Primitive = Cast<const PrimitiveTypeNode>(Node))
            return Primitive->Type;
        if (const auto Ptr = Cast<const PointerTypeNode>(Node))
            return CreatePtr(CreateFromAST(Ptr->BaseType, TypesArena), TypesArena);
        // if (const auto Ref = Cast<const ReferenceTypeNode>(Node))
        //
        if (const auto Array = Cast<const ArrayTypeNode>(Node))
        {
            if (auto Int = Cast<IntegerNode>(Array->Length))
            {
                return CreateArray(CreateFromAST(
                    Array->BaseType, TypesArena), Int->Value, TypesArena);
            }

            return nullptr;
        }

        return nullptr;
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
        if (const auto ArrType = Cast<const ArrayType>(Type))
            return llvm::ArrayType::get(GetLLVMType(ArrType->BaseType, Context), ArrType->Length);
        if (Cast<const PointerType>(Type))
            return llvm::PointerType::get(Context, 0);
        if (Cast<const ReferenceType>(Type))
            return llvm::PointerType::get(Context, 0);

        throw std::runtime_error("Unsupported data type");
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

        if (const auto LeftArrType = Cast<const ArrayType>(Left))
            if (const auto RightArrType = Cast<const ArrayType>(Right))
                return LeftArrType->Length == RightArrType->Length &&
                    IsEqual(LeftArrType->BaseType, RightArrType->BaseType);

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

        if (Cast<const CharType>(Type))
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

        if (Cast<CharType>(Type))
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

    std::string DataType::ToString() const
    {
        return TypeToString(Type);
    }

    std::string DataType::TypeToString(DataTypeBase *Type)
    {
        if (Cast<const VoidType>(Type))
            return "void";
        if (Cast<const BoolType>(Type))
            return "bool";
        if (Cast<const CharType>(Type))
            return "char";
        if (const auto Integer = Cast<const IntegerType>(Type))
        {
            switch (Integer->BitWidth)
            {
                case 8:  return "byte";
                case 32: return "int";
                case 64: return "long";
                default: throw std::runtime_error("Unsupported integer size");
            }
        };
        if (const auto Float = Cast<const FloatingPointType>(Type))
        {
            switch (Float->BitWidth) {
                case 32: return "float";
                case 64: return "double";
                case 128: return "float128";
                default: throw std::runtime_error("Unsupported FP size");
            }
        }
        if (const auto PtrType = Cast<const PointerType>(Type))
            return TypeToString(PtrType->BaseType) + "*";
        if (const auto RefType = Cast<const ReferenceType>(Type))
            return TypeToString(RefType->BaseType) + "$";

        throw std::runtime_error("Unsupported data type");
    }
}
