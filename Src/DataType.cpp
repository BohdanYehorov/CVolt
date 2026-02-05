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
        if (Cast<const VoidType>(Type)) return 0;
        if (Cast<const BoolType>(Type)) return 1;
        if (Cast<const CharType>(Type)) return 2;

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

    TypeCategory DataType::GetTypeCategory(DataTypeBase *Type)
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

        if (const auto ArrType = Cast<const ArrayType>(Type))
            return TypeToString(ArrType->BaseType) + "[" + std::to_string(ArrType->Length) + "]";

        throw std::runtime_error("Unsupported data type");
    }
}
