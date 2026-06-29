//
// Created by bohdan on 21.06.26.
//

#include "Volt/Core/Types/ClassType.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"

namespace Volt
{
    llvm::Type *ClassType::ToLLVMType(llvm::LLVMContext &Context) const
    {
        SmallVec4<llvm::Type*> Types;
        Types.reserve(Fields.Length());
        for (auto Field : Fields)
            Types.push_back(Field.Type->ToLLVMType(Context));

        return llvm::StructType::create(Context, Types, Name);
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
        if (Fields.Empty())
        {
            Size = 1;
            Alignment = 1;
            return;
        }

        for (const auto& [_, Type] : Fields)
        {
            size_t FieldSize = Type->GetSize();
            size_t FieldAlign = Type->GetAlignment();

            Size = AlignUp(Size, FieldAlign) + FieldSize;
            Alignment = std::max(Alignment, FieldAlign);
        }
        Size = AlignUp(Size, Alignment);
    }

    size_t ClassType::GetFieldIndex(const std::string &Name)
    {
        for (size_t i = 0; i < Fields.Length(); i++)
        {
            if (Fields[i].Name == Name)
                return i;
        }

        return Fields.Length();
    }
}
