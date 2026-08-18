//
// Created by bohdan on 21.06.26.
//

#include "Volt/Core/Types/ClassType.h"
#include <iostream>
#include <queue>

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

    size_t ClassType::GetImplementedFieldOffset(ClassType *Owner) const
    {
        if (this == Owner) return 0;

        std::queue<std::pair<ClassType*, size_t>> FieldsQueue;
        for (auto [Type, Index] : ImplementedClassTypes)
        {
            if (Type == Owner)
                return Fields[Index].Offset;
            FieldsQueue.emplace(Type, Fields[Index].Offset);
        }

        while (!FieldsQueue.empty())
        {
            const auto& F = FieldsQueue.front();
            FieldsQueue.pop();

            ClassType* CurType = F.first;
            size_t CurOffset = F.second;

            for (auto [Type, Index] : CurType->ImplementedClassTypes)
            {
                size_t NewOffset = CurOffset + CurType->Fields[Index].Offset;
                if (Type == Owner)
                    return NewOffset;
                FieldsQueue.emplace(Type, NewOffset);
            }
        }

        VoltUnreachableFmt("This class {} not implemented to {}", ToString(), Owner->ToString());
    }

    void ClassType::ImplementField(size_t FieldIndex)
    {
        VoltAssert(FieldIndex < Fields.Length());
        const Field& F = Fields[FieldIndex];
        auto ClassTy = F.Type.CastAs<ClassType>();
        VoltAssert(ClassTy && "Cannot implement field with non-class type");

        for (const auto& [Name, Overload] : ClassTy->Methods)
        {
            Methods.AddFunction(Name, Overload.Args, Overload.Callee, ClassTy);
            ImplementedClassTypes[ClassTy] = FieldIndex;
        }
    }
}
