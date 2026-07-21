//
// Created by bohdan on 20.07.26.
//

#include "Volt/Core/Types/ClassInst.h"

namespace Volt
{
    void *ClassInstBase::GetField(llvm::StringRef FieldName, size_t Size) const
    {
        size_t FieldIndex = Type->GetFieldIndex(FieldName.str());
        VoltAssert(FieldIndex != Type->Fields.Length());
        const Field& F = Type->Fields[FieldIndex];
        VoltAssert(F.Type->GetSize() == Size);
        return Data + F.Offset;
    }

    ClassInstView ClassInstBase::GetField(llvm::StringRef FieldName) const
    {
        size_t FieldIndex = Type->GetFieldIndex(FieldName.str());
        VoltAssert(FieldIndex != Type->Fields.Length());
        const Field& F = Type->Fields[FieldIndex];
        VoltAssert(F.Type->IsClassType());
        return { F.Type.CastAs<ClassType>(), Data + F.Offset,  CContext };
    }

    ClassInst::ClassInst(const ClassInst &Other)
        : ClassInstBase(Other.Type, Other.CContext)
    {
        size_t Size = Other.Type->GetSize();
        Data = static_cast<char *>(operator new(Size));
        std::memcpy(Data, Other.Data, Size);
    }

    ClassInst& ClassInst::operator=(const ClassInst &Other)
    {
        if (this != &Other)
        {
            size_t OldSize = Type->GetSize();
            size_t NewSize = Other.Type->GetSize();

            if (OldSize < NewSize)
            {
                operator delete(Data);
                Data = static_cast<char *>(operator new(NewSize));
            }

            std::memcpy(Data, Other.Data, NewSize);
            Type = Other.Type;
        }

        return *this;
    }

    ClassInst::ClassInst(ClassInst &&Other) noexcept
        : ClassInstBase(Other.Type, Other.CContext)
    {
        Data = Other.Data;

        Other.Type = nullptr;
        Other.Data = nullptr;
    }

    ClassInst & ClassInst::operator=(ClassInst &&Other) noexcept
    {
        if (this != &Other)
        {
            if (Data) operator delete(Data);

            Type = Other.Type;
            Data = Other.Data;

            Other.Type = nullptr;
            Other.Data = nullptr;
        }

        return *this;
    }
}
