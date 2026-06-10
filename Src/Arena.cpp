//
// Created by bohdan on 14.12.25.
//

#include "Volt/Core/Memory/Arena.h"
#include <cstring>
#include <iostream>

namespace Volt
{
    void ArenaAllocator::Allocate(size_t InSize)
    {
        Size = InSize;
        Data = static_cast<std::byte*>(::operator new(InSize));
    }

    void ArenaAllocator::Deallocate()
    {
        if (!Data) return;
        ::operator delete(Data);
        Data = nullptr;
        Size = 0;
    }

    void* ArenaAllocator::Write(PtrT Ptr, const void* InData, size_t InSize, size_t Align)
    {
        size_t Padding = CalculatePadding(Align, Ptr);
        if (Padding != 0)
            VoltUnreachable("Ref is not aligned");

        std::memcpy(Data + Ptr, InData, InSize);

        UsedSize = std::max(UsedSize, Ptr + InSize);
        return Data + Ptr;
    }

    size_t ArenaAllocator::CalculatePadding(size_t Align, size_t Pos)
    {
        return (Align - (Pos % Align)) % Align;
    }

    void ArenaStream::Deallocate()
    {
        Alloc.Deallocate();
        WritePtr = 0;
        ReadPtr = 0;
    }

    void* ArenaStream::Write(const void *InData, size_t InSize, size_t Align)
    {
        WritePtr += ArenaAllocator::CalculatePadding(Align, WritePtr);
        void* Ptr = Alloc.Write(WritePtr, InData, InSize, Align);
        WritePtr += InSize;
        return Ptr;
    }

    StringRef ArenaStream::Write(const std::string &Str)
    {
        StringRef Ref = Alloc.Write(WritePtr, Str);
        WritePtr += Str.size();
        return Ref;
    }

    BufferStringView ArenaStream::Read(size_t Count) const
    {
        ReadPtr += ArenaAllocator::CalculatePadding(alignof(char), ReadPtr);
        BufferStringView Ptr = Alloc.Read(ReadPtr, Count);
        ReadPtr += sizeof(char) * Count;
        return Ptr;
    }
}
