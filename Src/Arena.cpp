//
// Created by bohdan on 14.12.25.
//

#include "Arena.h"
#include <cstring>
#include <iostream>

void ArenaAllocator::Allocate(size_t InSize)
{
    Size = InSize;
    Data = static_cast<std::byte*>(::operator new(InSize));
}

void ArenaAllocator::Reallocate(size_t NewSize)
{
    size_t MinSize = std::min(UsedSize, NewSize);
    auto NewData = static_cast<std::byte*>(::operator new(NewSize));
    if (Size > 0)
    {
        std::memcpy(NewData, Data, MinSize);
        ::operator delete(Data, Size);
    }

    Data = NewData;
    Size = NewSize;
    UsedSize = MinSize;
}

void ArenaAllocator::Deallocate()
{
    if (!Data) return;
    ::operator delete(Data, Size);
    Data = nullptr;
    Size = 0;
}

void* ArenaAllocator::Write(PtrT Ptr, const void* InData, size_t InSize, size_t Align)
{
    size_t Padding = CalculatePadding(Align, Ptr);
    if (Padding != 0)
        throw std::runtime_error("Ref is not aligned");

    ReallocateOrThrow(Ptr + InSize);

    std::memcpy(Data + Ptr, InData, InSize);

    UsedSize = std::max(UsedSize, Ptr + InSize);
    return Data + Ptr;
}

size_t ArenaAllocator::CalculatePadding(size_t Align, size_t Pos)
{
    return (Align - (Pos % Align)) % Align;
}

void ArenaAllocator::ReallocateOrThrow(size_t NewSize)
{
    if (NewSize > Size)
    {
        if (!AutoReallocate)
            throw std::runtime_error("Cannot Write Data");

        Reallocate(CalculateCapacity(NewSize));
    }
}

void ArenaStream::Reallocate(size_t NewSize)
{
    Alloc.Reallocate(NewSize);
    if (WritePtr > NewSize)
        WritePtr = NewSize;
    if (ReadPtr > NewSize)
        ReadPtr = NewSize;
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
