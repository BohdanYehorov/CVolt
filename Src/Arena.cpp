//
// Created by bohdan on 14.12.25.
//

#include "Volt/Core/Memory/Arena.h"
#include <cstring>
#include <iostream>

namespace Volt
{
    ArenaAllocator & ArenaAllocator::operator=(ArenaAllocator &&Other) noexcept
    {
        if (this != &Other)
        {
            if (Data)
                operator delete(Data);

            Data = Other.Data;
            Size = Other.Size;
            Pos = Other.Pos;

            Other.Data = nullptr;
            Other.Size = 0;
            Other.Pos = 0;
        }

        return *this;
    }

    void * ArenaAllocator::Alloc(size_t AllocSize, size_t Align)
    {
        size_t Padding = CalculatePadding(Align);
        assert(Pos + Padding + AllocSize <= Size);

        Pos += Padding;
        void *Ptr = Data + Pos;
        Pos += AllocSize;
        return Ptr;
    }

    bool ArenaAllocator::CanFit(size_t AllocSize, size_t Align) const
    {
        return Pos + CalculatePadding(Align) + AllocSize <= Size;
    }

    Arena::~Arena()
    {
        for (auto& D : Destructors)
            D.Destructor(D.Obj, D.Count);
    }

    void * Arena::Alloc(size_t Size, size_t Align)
    {
        if (Blocks.empty() || !Blocks.back().CanFit(Size, Align))
            Blocks.emplace_back(BlockSize);

        return Blocks.back().Alloc(Size, Align);
    }
}
