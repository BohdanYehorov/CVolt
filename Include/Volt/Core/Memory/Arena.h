//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_ARENA_H
#define CVOLT_ARENA_H

#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include "Volt/Core/Memory/BufferView.h"
#include "Volt/ADT/Array.h"
#include "Volt/Core/Object/Object.h"
#include <stdexcept>
#include <iostream>

namespace Volt
{
    class ArenaAllocator
    {
    private:
        size_t Size = 0;
        size_t UsedSize = 0;
        std::byte* Data = nullptr;

    public:
        [[nodiscard]] size_t GetSize() const { return Size; }
        [[nodiscard]] size_t GetUsedSize() const { return UsedSize; }
        [[nodiscard]] void* GetData() const { return Data; }

        ~ArenaAllocator() { Deallocate(); }

        void Allocate(size_t InSize);
        void Deallocate();

        template <typename T, typename ...Args_>
        T* Construct(PtrT Ptr, Args_&&... Args);

        void* Write(PtrT Ptr, const void* InData, size_t InSize, size_t Align = 1);

        template <typename T>
        T* Write(PtrT Ptr, const T* InData, size_t Count);

        template <typename T>
        T* Read(PtrT Ptr) const;

        StringRef Write(PtrT Ptr, const std::string& Str)
        {
            Write<char>(Ptr, Str.c_str(), Str.size());
            return { Ptr, Str.size() };
        }

        template<typename T>
        BufferArrayView<T> Read(PtrT Ptr, size_t Count) const;
        [[nodiscard]] BufferStringView Read(PtrT Ptr, size_t Count) const { return BufferStringView{ Data + Ptr, Count }; }
        [[nodiscard]] BufferStringView Read(StringRef Ref) const { return Read(Ref.Ptr, Ref.Length); }

    private:
        static size_t CalculatePadding(size_t Align, size_t Pos);
        static size_t CalculateCapacity(size_t InSize) { return static_cast<size_t>(static_cast<float>(InSize) * 1.5f); }

        friend class ArenaStream;
    };

    class ArenaStream
    {
    private:
        ArenaAllocator Alloc;
        PtrT WritePtr = 0;
        mutable PtrT ReadPtr = 0;

    public:
        ArenaStream() = default;
        ArenaStream(size_t Size) { Alloc.Allocate(Size); }

        [[nodiscard]] size_t GetSize() const { return Alloc.GetSize(); }
        [[nodiscard]] size_t GetUsedSize() const { return Alloc.GetUsedSize(); }
        [[nodiscard]] void* GetData() const { return Alloc.GetData(); }
        [[nodiscard]] PtrT GetWritePtr() const { return WritePtr; }
        [[nodiscard]] PtrT GetReadPtr() const { return ReadPtr; }
        [[nodiscard]] ArenaAllocator& GetArenaAllocator() { return Alloc; }
        [[nodiscard]] const ArenaAllocator& GetArenaAllocator() const { return Alloc; }

        template <typename T>
        [[nodiscard]] bool CanWrite() const;

        ~ArenaStream() { Deallocate(); }

        void Allocate(size_t InSize) { Alloc.Allocate(InSize); }
        void Deallocate();

        void SetWritePtr(PtrT NewWritePtr) { WritePtr = NewWritePtr; }
        void SetReadPtr(PtrT NewReadPtr) const { ReadPtr = NewReadPtr; }

        template <typename T, typename ...Args_>
        T* Construct(Args_&&... Args);

        void* Write(const void* InData, size_t InSize, size_t Align = 1);

        template <typename T>
        T* Write(const T* InData, size_t Count);

        StringRef Write(const std::string& Str);

        template <typename T>
        [[nodiscard]] T* Read();

        template<typename T>
        [[nodiscard]] BufferArrayView<T> Read(size_t Count) const;
        [[nodiscard]] BufferStringView Read(size_t Count) const;
        [[nodiscard]] BufferStringView Read(StringRef Ref) const { return Alloc.Read(Ref.Ptr, Ref.Length); }
    };

    class Arena
    {
        struct DestructorEntry
        {
            Object* Obj;
            void (*Destroy)(void*);

            DestructorEntry(Object* Obj, void (*Destroy)(void*))
                : Obj(Obj), Destroy(Destroy) {}
        };

    private:
        size_t BlockSize = 64 * 1024;
        Array<ArenaStream> Blocks;
        Array<DestructorEntry> Objects;

    public:
        Arena() = default;
        Arena(size_t BlockSize) : BlockSize(BlockSize) {}

        ~Arena()
        {
            for (const auto& Entry : Objects)
                Entry.Destroy(Entry.Obj);
        }

        template <typename T, typename ...Args_>
        std::enable_if_t<std::is_base_of_v<Object, T>, T*>
        Create(Args_&&... Args);

    private:
        template<typename T>
        static void Destroy(void* Obj)
        {
            static_cast<T*>(Obj)->~T();
        }
    };

    template<typename T, typename ... Args_>
    T* ArenaAllocator::Construct(PtrT Ptr, Args_&&... Args)
    {
        size_t Padding = CalculatePadding(alignof(T), Ptr);

        if (Padding != 0)
            throw std::runtime_error("Ref Is Not Aligned");

        UsedSize = std::max(UsedSize, Ptr + sizeof(T));
        return new (Data + Ptr) T(std::forward<Args_>(Args)...);
    }

    template<typename T>
    T* ArenaAllocator::Write(PtrT Ptr, const T* InData, size_t Count)
    {
        static_assert(std::is_trivially_copyable_v<T>,
                  "ArenaAllocator::Init requires trivially copyable type");

        return static_cast<T*>(Write(Ptr, InData, sizeof(T) * Count, alignof(T)));
    }

    template<typename T>
    T* ArenaAllocator::Read(PtrT Ptr) const
    {
        size_t Padding = CalculatePadding(alignof(T), Ptr);

        if (Padding != 0)
            throw std::runtime_error("Ref Is Not Aligned");

        if (Ptr + sizeof(T) > Size)
            throw std::runtime_error("Cannot Get This Object");

        return std::launder(reinterpret_cast<T*>(Data + Ptr));
    }

    template<typename T>
    BufferArrayView<T> ArenaAllocator::Read(PtrT Ptr, size_t Count) const
    {
        return BufferArrayView<T>(Data + Ptr, Count);
    }

    template<typename T>
    bool ArenaStream::CanWrite() const
    {
        return sizeof(T) + WritePtr + ArenaAllocator::CalculatePadding(alignof(T), WritePtr) <= GetSize();
    }

    template<typename T, typename ... Args_>
    T* ArenaStream::Construct(Args_&&... Args)
    {
        WritePtr += ArenaAllocator::CalculatePadding(alignof(T), WritePtr);
        T* Ptr = Alloc.Construct<T, Args_...>(WritePtr, std::forward<Args_>(Args)...);
        WritePtr += sizeof(T);
        return Ptr;
    }

    template<typename T>
    T* ArenaStream::Write(const T *InData, size_t Count)
    {
        WritePtr += ArenaAllocator::CalculatePadding(alignof(T), WritePtr);
        T* Ptr = Alloc.Write(WritePtr, InData, Count);
        WritePtr += sizeof(T) * Count;
        return Ptr;
    }

    template<typename T>
    T* ArenaStream::Read()
    {
        ReadPtr += ArenaAllocator::CalculatePadding(alignof(T), ReadPtr);
        T* Ptr = Alloc.Read<T>(ReadPtr);
        ReadPtr += sizeof(T);
        return Ptr;
    }

    template<typename T>
    BufferArrayView<T> ArenaStream::Read(size_t Count) const
    {
        ReadPtr += ArenaAllocator::CalculatePadding(alignof(T), ReadPtr);
        BufferArrayView<T> Ptr = Alloc.Read<T>(ReadPtr, Count);
        ReadPtr += sizeof(T) * Count;
        return Ptr;
    }

    template<typename T, typename ... Args_>
    std::enable_if_t<std::is_base_of_v<Object, T>, T*> Arena::Create(Args_ &&...Args)
    {
        assert(sizeof(T) <= BlockSize);

        if (Blocks.Empty() || !Blocks.Back().CanWrite<T>())
            Blocks.Emplace(BlockSize);

        T* Obj = Blocks.Back().Construct<T>(std::forward<Args_>(Args)...);
        if constexpr (!std::is_trivially_destructible_v<T>)
            Objects.Emplace(Obj, Destroy<T>);

        return Obj;
    }
}

#endif //CVOLT_ARENA_H
