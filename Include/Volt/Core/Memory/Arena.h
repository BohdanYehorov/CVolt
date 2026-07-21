//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_ARENA_H
#define CVOLT_ARENA_H

#include "Volt/Core/Object/Object.h"

namespace Volt
{
    class ArenaAllocator
    {
        char *Data;
        size_t Size;
        size_t Pos;

    public:
        ArenaAllocator(size_t Size) : Size(Size), Pos(0)
        {
            Data = static_cast<char *>(operator new(Size));
        }

        ArenaAllocator(const ArenaAllocator&) = delete;
        ArenaAllocator &operator=(const ArenaAllocator&) = delete;

        ArenaAllocator(ArenaAllocator&& Other) noexcept
            : Data(nullptr), Size(0), Pos(0)
        {
            std::swap(Data, Other.Data);
            std::swap(Size, Other.Size);
            std::swap(Pos, Other.Pos);
        }

        ArenaAllocator &operator=(ArenaAllocator&& Other) noexcept;

        ~ArenaAllocator() { operator delete(Data); }

        void* Alloc(size_t AllocSize, size_t Align);

        [[nodiscard]] bool CanFit(size_t Size, size_t Align) const;

    private:
        [[nodiscard]] size_t CalculatePadding(size_t Align) const
        {
            return CalculatePadding(Align, GetGlobalPos());
        }

        [[nodiscard]] static size_t CalculatePadding(size_t Align, size_t Pos)
        {
            size_t Mask = Align - 1;
            return (Align - (Pos & Mask)) & Mask;
        }

        [[nodiscard]] uintptr_t GetGlobalPos() const { return reinterpret_cast<uintptr_t>(Data) + Pos; }
    };

    class Arena
    {
        std::vector<ArenaAllocator> Blocks;
        std::vector<std::pair<void(*)(void*), void*>> Destructors;

        size_t BlockSize = 64 * 1024;

    public:
        ~Arena();

        template <typename T, typename ...Args_>
        T *Create(Args_&&... Args);
        void *Alloc(size_t Size, size_t Align);
    };

    template<typename T, typename ... Args_>
    T *Arena::Create(Args_ &&...Args)
    {
        T *Ptr = static_cast<T*>(Alloc(sizeof(T), alignof(T)));
        new (Ptr) T(std::forward<Args_>(Args)...);

        if (!std::is_trivially_destructible_v<T>)
        {
            Destructors.push_back(std::make_pair([](void* Ptr) {
                static_cast<T*>(Ptr)->~T();
            }, Ptr));
        }

        return Ptr;
    }
}

#endif //CVOLT_ARENA_H
