//
// Created by bohdan on 21.12.25.
//

#ifndef CVOLT_BUFFERVIEW_H
#define CVOLT_BUFFERVIEW_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include "Volt/Core/Types/IntTypeDefs.h"

namespace Volt
{
    template <typename T>
    class BufferArrayView
    {
    private:
        std::byte* Ptr;
        size_t Count;

    public:
        BufferArrayView(std::byte* Ptr, size_t Count);

        [[nodiscard]] T& operator[](size_t Index);
        [[nodiscard]] const T& operator[](size_t Index) const;

        [[nodiscard]] size_t Size() const { return Count; }
        [[nodiscard]] std::byte* Data() const { return Ptr; }

        [[nodiscard]] T* Begin() { return reinterpret_cast<T*>(Ptr); }
        [[nodiscard]] T* End() { return reinterpret_cast<T*>(Ptr) + Count; }
        [[nodiscard]] const T* CBegin() const { return reinterpret_cast<T*>(Ptr); }
        [[nodiscard]] const T* CEnd() const { return reinterpret_cast<T*>(Ptr) + Count; }

        [[nodiscard]] T* begin() { return Begin(); }
        [[nodiscard]] T* end()   { return End(); }
        [[nodiscard]] const T* begin() const { return CBegin(); }
        [[nodiscard]] const T* end() const   { return CEnd(); }
    };

    class BufferStringView : public BufferArrayView<char>
    {
    public:
        BufferStringView(std::byte* Ptr, size_t Count) : BufferArrayView(Ptr, Count) {}

        [[nodiscard]] bool operator==(const BufferStringView& Other) const;
        [[nodiscard]] char* CStr() const { return reinterpret_cast<char*>(Data()); }
        [[nodiscard]] std::string ToString() const
        { return std::string{ reinterpret_cast<const char*>(Data()), Size() }; }
    };

    struct StringRef
    {
        PtrT Ptr = 0;
        size_t Length = 0;

        StringRef() = default;
        StringRef(PtrT Ptr, size_t Length)
            : Ptr(Ptr), Length(Length) {}
    };

    template<typename T>
    BufferArrayView<T>::BufferArrayView(std::byte *Ptr, size_t Count) : Ptr(Ptr), Count(Count)
    {
        if (reinterpret_cast<uintptr_t>(Ptr) % alignof(T))
            throw std::runtime_error("Unaligned Buffer");
    }

    template<typename T>
    T& BufferArrayView<T>::operator[](size_t Index)
    {
        if (Index >= Count)
            throw std::range_error("Index Out Of Range");
        return *reinterpret_cast<T*>(Ptr + Index * sizeof(T));
    }

    template<typename T>
    const T& BufferArrayView<T>::operator[](size_t Index) const
    {
        if (Index >= Count)
            throw std::range_error("Index Out Of Range");
        return *reinterpret_cast<T*>(Ptr + Index * sizeof(T));
    }

    bool operator==(const BufferStringView& Left, const std::string& Right);
    bool operator==(const std::string& Left, const BufferStringView& Right);
}

#endif //CVOLT_BUFFERVIEW_H
