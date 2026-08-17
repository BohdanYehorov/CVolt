//
// Created by bohdan on 21.06.26.
//

#include "Volt/Core/Types/ClassType.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include <iostream>

namespace Volt
{
    llvm::Type *ClassType::ToLLVMType(llvm::LLVMContext &Context) const
    {
        return llvm::ArrayType::get(
            llvm::Type::getInt8Ty(Context), GetSize());
    }

    size_t ClassType::GetSize() const
    {
        if (Size == 0)
            ComputeLayout();
        return Size;
    }

    size_t ClassType::GetAlignment() const
    {
        if (Alignment == 0)
            ComputeLayout();
        return Alignment;
    }

    void ClassType::ComputeLayout() const
    {
        VoltAssert(ClassInitialized && "Cannot compute layout of non-initialized class");

        if (Fields.Empty())
        {
            Size = 1;
            Alignment = 1;
            return;
        }

        for (const auto& [_, Type, Offset] : Fields)
        {
            size_t FieldSize = Type->GetSize();
            size_t FieldAlign = Type->GetAlignment();

            size_t FieldOffset = AlignUp(Size, FieldAlign);
            const_cast<size_t&>(Offset) = FieldOffset;
            Size = FieldOffset + FieldSize;
            Alignment = std::max(Alignment, FieldAlign);
        }
        Size = AlignUp(Size, Alignment);
    }

    size_t ClassType::GetFieldIndex(llvm::StringRef Name)
    {
        VoltAssert(ClassInitialized && "Cannot get field from non-initialized class");

        for (size_t i = 0; i < Fields.Length(); i++)
            if (Fields[i].Name == Name)
                return i;

        return Fields.Length();
    }
}
