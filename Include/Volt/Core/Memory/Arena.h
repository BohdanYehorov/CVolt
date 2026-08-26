//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_ARENA_H
#define CVOLT_ARENA_H

#include "Volt/ADT/Array.h"

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
        ArenaAllocator(void* Data, size_t Size, size_t Pos)
            : Data(static_cast<char *>(Data)), Size(Size), Pos(Pos) {}

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

        friend class Arena;
    };

    class Arena
    {
        struct DestructorEntry
        {
            void(*Destructor)(void*, size_t);
            void* Obj;
            size_t Count;

            DestructorEntry(void(*Destructor)(void*, size_t), void* Obj, size_t Count)
                : Destructor(Destructor), Obj(Obj), Count(Count) {}
        };

        std::vector<ArenaAllocator> Blocks;
        std::vector<DestructorEntry> Destructors;

        size_t BlockSize = 64 * 1024;

    public:
        ~Arena();

        template <typename T, typename ...Args_>
        T *Create(Args_&&... Args);
        void *Alloc(size_t Size, size_t Align);

        template <typename T>
        llvm::ArrayRef<T> AllocStaticArray(Array<T>&& Arr);
    };

    template<typename T, typename ... Args_>
    T *Arena::Create(Args_ &&...Args)
    {
        T *Ptr = static_cast<T*>(Alloc(sizeof(T), alignof(T)));
        new (Ptr) T(std::forward<Args_>(Args)...);

        if (!std::is_trivially_destructible_v<T>)
        {
            Destructors.emplace_back([](void* Ptr, size_t) {
                static_cast<T*>(Ptr)->~T();
            }, Ptr, 1);
        }

        return Ptr;
    }

    template<typename T>
    llvm::ArrayRef<T> Arena::AllocStaticArray(Array<T> &&Arr)
    {
        size_t Len = Arr.Len;
        T* Data = Arr.Data;
        ArenaAllocator Alloc(Data, Arr.Cap, Len);
        Arr.Reset();

        ArenaAllocator& Block = Blocks.emplace_back(std::move(Alloc));

        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            Destructors.emplace_back([](void* Ptr, size_t Len) {
                for (size_t i = 0; i < Len; i++)
                    (static_cast<T*>(Ptr) + i)->~T();
            }, Block.Data, Len);
        }

        return llvm::ArrayRef<T>(Data, Len);
    }
}

#endif //CVOLT_ARENA_H
