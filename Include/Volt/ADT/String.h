//
// Created by bohdan on 08.02.26.
//

#ifndef CVOLT_STRING_H
#define CVOLT_STRING_H

#include "ArrayAllocator.h"
#include <cstring>

namespace Volt
{
    template<typename Alloca = ArrayAllocator<char>>
    class StringImpl
    {
    public:
        using SizeType = size_t;
        using DifferenceType = std::ptrdiff_t;
        using AllocatorType = Alloca;

    private:
        char* Data = nullptr;
        SizeType Len = 0;
        SizeType Cap = 0;
        AllocatorType Alloc;

    public:
        StringImpl() = default;
        StringImpl(std::nullptr_t) = delete;
        StringImpl(const char* Str)
        {
            if (!Str) return;
            SizeType StrLen = std::strlen(Str);
            if (StrLen == 0) return;
            RawResize(StrLen);
            std::memcpy(Data, Str, StrLen);
        }
        StringImpl(std::string_view Str)
        {
            if (Str.empty()) return;
            RawResize(Str.size());
            std::memcpy(Data, Str.data(), Str.size());
        }
        StringImpl(llvm::StringRef Ref)
        {
            if (Ref.empty()) return;
            RawResize(Ref.size());
            std::memcpy(Data, Ref.data(), Ref.size());
        }
        StringImpl(SizeType Len, char Fill) { Resize(Len, Fill); }

        StringImpl(const StringImpl& Other)
        {
            if (!Other.Data) return;

            Len = Other.Len;
            Cap = Other.Cap;
            Data = Alloc.Allocate(Cap);
            std::memcpy(Data, Other.Data, Len);
            Data[Len] = '\0';
        }
        StringImpl(StringImpl&& Other) noexcept
        {
            std::swap(Len, Other.Len);
            std::swap(Cap, Other.Cap);
            std::swap(Data, Other.Data);
        }

        StringImpl& operator=(const StringImpl& Other)
        {
            if (this != &Other)
            {
                if (!Other.Data)
                {
                    if (Data) Data[0] = '\0';
                    Len = 0;
                    return *this;
                }
                if (Other.Len + 1 > Cap)
                {
                    char* NewData = Alloc.Allocate(Other.Len + 1);
                    if (Data) Alloc.Deallocate(Data);
                    Data = NewData;
                    Cap = Other.Len + 1;
                }
                std::memcpy(Data, Other.Data, Other.Len);
                Data[Other.Len] = '\0';
                Len = Other.Len;
            }

            return *this;
        }

        StringImpl& operator=(StringImpl&& Other) noexcept
        {
            if (this != &Other)
            {
                if (Data) Alloc.Deallocate(Data);
                Len = Other.Len;
                Cap = Other.Cap;
                Data = Other.Data;

                Other.Len = Other.Cap = 0;
                Other.Data = nullptr;
            }

            return *this;
        }

        ~StringImpl() { Alloc.Deallocate(Data); }

        [[nodiscard]] char& operator[](SizeType Index) { return Data[Index]; }
        [[nodiscard]] char operator[](SizeType Index) const { return Data[Index]; }

        [[nodiscard]] SizeType Length() const { return Len; }
        [[nodiscard]] SizeType Capacity() const { return Cap; }
        [[nodiscard]] char* RawData() { return Data; }
        [[nodiscard]] const char* RawData() const { return Data; }
        [[nodiscard]] const char* CStr() const { return Data; }

        [[nodiscard]] bool Empty() const { return Len == 0; }

        void Add(char Ch)
        {
            if (Len + 1 >= Cap) Reserve(CalculateCapacity(Cap));
            Data[Len] = Ch;
            Data[Len + 1] = '\0';
            Len++;
        }

        void Pop()
        {
            if (Len == 0) return;
            Len--;
            Data[Len] = '\0';
        }

        void Reserve(SizeType NewCap)
        {
            if (NewCap <= Cap) return;
            char* NewData = Alloc.Allocate(NewCap);
            if (Data)
            {
                std::memcpy(NewData, Data, Len);
                Alloc.Deallocate(Data);
            }
            NewData[Len] = '\0';
            Data = NewData;
            Cap = NewCap;
        }

        void Resize(SizeType NewLen, char Fill = ' ')
        {
            SizeType OldLen = Len;
            RawResize(NewLen);
            if (Len > OldLen)
                std::memset(Data + OldLen, Fill, NewLen - OldLen);
        }
    private:
        static SizeType CalculateCapacity(SizeType InCap)
        {
            if (InCap < 32) return 32;
            return SizeType(1) << (static_cast<SizeType>(std::bit_width(InCap)));
        }

        void RawResize(SizeType NewLen)
        {
            if (NewLen == Len) return;
            if (NewLen > Len)
                Reserve(CalculateCapacity(NewLen + 1));
            Data[NewLen] = '\0';
            Len = NewLen;
        }
    };

    using String = StringImpl<ArrayAllocator<char>>;
}

#endif //CVOLT_STRING_H