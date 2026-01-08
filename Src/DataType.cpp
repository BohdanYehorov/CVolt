//
// Created by bohdan on 08.01.26.
//

#include "DataType.h"
#include <llvm/IR/DerivedTypes.h>

namespace Volt
{
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

    bool DataType::IsVoidType() const
    {
        if (auto PrimitiveType = Cast<PrimitiveDataTypeNode>(Type))
            return PrimitiveType->PrimitiveType == PrimitiveDataType::VOID;

        return false;
    }

    bool DataType::IsIntegerType() const
    {
        if (auto PrimitiveType = Cast<PrimitiveDataTypeNode>(Type))
            return PrimitiveType->PrimitiveType == PrimitiveDataType::BOOL ||
                   PrimitiveType->PrimitiveType == PrimitiveDataType::CHAR ||
                   PrimitiveType->PrimitiveType == PrimitiveDataType::BYTE ||
                   PrimitiveType->PrimitiveType == PrimitiveDataType::INT ||
                   PrimitiveType->PrimitiveType == PrimitiveDataType::LONG;

        return false;
    }

    bool DataType::IsFloatingPointType() const
    {
        if (auto PrimitiveType = Cast<PrimitiveDataTypeNode>(Type))
            return PrimitiveType->PrimitiveType == PrimitiveDataType::FLOAT ||
                   PrimitiveType->PrimitiveType == PrimitiveDataType::DOUBLE;

        return false;
    }

    int DataType::GetTypeBitWidth() const
    {
        if (auto PrimitiveType = Cast<PrimitiveDataTypeNode>(Type))
        {
            switch (PrimitiveType->PrimitiveType)
            {
                case PrimitiveDataType::VOID:   return 0;
                case PrimitiveDataType::BOOL:   return 1;
                case PrimitiveDataType::CHAR:
                case PrimitiveDataType::BYTE:   return 8;
                case PrimitiveDataType::INT:    return 32;
                case PrimitiveDataType::LONG:   return 64;
                case PrimitiveDataType::FLOAT:  return 32;
                case PrimitiveDataType::DOUBLE: return 64;
                default: return -1;
            }
        }

        return -1;
    }

    llvm::Type* DataType::GetLLVMType(const DataTypeNodeBase* Type, llvm::LLVMContext &Context)
    {
        if (const auto PrimitiveType = Cast<const PrimitiveDataTypeNode>(Type))
            return ToLLVMPrimitiveType(PrimitiveType->PrimitiveType, Context);
        if (const auto PtrType = Cast<const PtrDataTypeNode>(Type))
            return llvm::PointerType::get(
                GetLLVMType(PtrType->BaseType, Context)->getContext(), 0);
        if (const auto RefType = Cast<const RefDataTypeNode>(Type))
            return llvm::PointerType::get(
                GetLLVMType(RefType->BaseType, Context)->getContext(), 0);

        return nullptr;
    }
}