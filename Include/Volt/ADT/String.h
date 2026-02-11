//
// Created by bohdan on 08.02.26.
//

#ifndef CVOLT_STRING_H
#define CVOLT_STRING_H

#include "ArrayAllocator.h"
#include "ArrayIterator.h"
#include <cstring>

namespace Volt
{
	struct RelativeStringRef
	{
		size_t Pos;
		size_t Length;
	};

	template <typename T, typename Alloca = ArrayAllocator<T>>
	class BasicString
	{
		using ValueType = T;
		using SizeType = size_t;
		using DifferenceType = std::ptrdiff_t;
		using AllocatorType = Alloca;
		using Iterator = ArrayIterator<T>;
		using ConstIterator = ArrayIterator<const T>;

	private:
		T* Str = nullptr;
		SizeType Len = 0;
		SizeType Cap = 0;
		AllocatorType Alloc;

	public:
		BasicString() = default;
		BasicString(SizeType Count, ValueType Ch);
		BasicString(const T* InStr);

		BasicString(std::nullptr_t) = delete;

		BasicString(const BasicString& Other);
		BasicString(BasicString&& Other) noexcept;

		template <typename Iter = Iterator>
		BasicString(Iter Begin, Iter End);

		BasicString& operator=(const BasicString& Other);
		BasicString& operator=(BasicString&& Other) noexcept;

		~BasicString() { Alloc.Deallocate(Str); }

		void Reserve(SizeType NewCap);

		void Add(T Ch);
		void Append(const BasicString& Other);

		[[nodiscard]] const ValueType& operator[](SizeType Index) const { return Str[Index]; }
		[[nodiscard]] ValueType& operator[](SizeType Index) { return Str[Index]; }

		[[nodiscard]] ValueType* RawData() const { return Str; }
		[[nodiscard]] const ValueType* CStr() const { return Str; }

		[[nodiscard]] SizeType Length() const { return Len; }
		[[nodiscard]] SizeType Capacity() const { return Cap; }
		[[nodiscard]] bool Empty() const { return Len == 0; }

		[[nodiscard]] ValueType& Front() { return Str[0]; }
		[[nodiscard]] const ValueType& Front() const { return Str[0]; }

		[[nodiscard]] ValueType& Back() { return Str[Len - 1]; }
		[[nodiscard]] const ValueType& Back() const { return Str[Len - 1]; }

		[[nodiscard]] Iterator Begin() { return Iterator(Str); }
		[[nodiscard]] Iterator End() { return Iterator(Str + Len); }

		[[nodiscard]] Iterator begin() { return Iterator(Str); }
		[[nodiscard]] Iterator end() { return Iterator(Str + Len); }

		[[nodiscard]] ConstIterator begin() const { return ConstIterator(Str); }
		[[nodiscard]] ConstIterator end()   const { return ConstIterator(Str + Len); }

		BasicString SubStr(RelativeStringRef Ref);

	public:
		void CreateRawString(SizeType InLen);
		void RawResize(SizeType NewLen);

		static SizeType StrLength(const T* Str);
		static SizeType CalculateCapacity(SizeType Len) { return Len * 2; }
		static void StrCopy(ValueType *Dst, const ValueType *Src, size_t Len);
	};

	template<typename T, typename Alloca>
	BasicString<T, Alloca>::BasicString(SizeType Count, ValueType Ch)
	{
		CreateRawString(Count);

		for (size_t i = 0; i < Count; i++)
			Str[i] = Ch;
	}

	template<typename T, typename Alloca>
	BasicString<T, Alloca>::BasicString(const T *InStr)
	{
		assert(InStr != nullptr);

		Len = StrLength(InStr);
		CreateRawString(Len);
		std::memcpy(Str, InStr, Len);
	}

	template<typename T, typename Alloca>
	BasicString<T, Alloca>::BasicString(const BasicString &Other)
	{
		CreateRawString(Other.Len);
		std::memcpy(Str, Other.Str, Other.Len * sizeof(ValueType));
	}

	template<typename T, typename Alloca>
	BasicString<T, Alloca>::BasicString(BasicString &&Other) noexcept
	{
		Str = Other.Str;
		Len = Other.Len;
		Cap = Other.Cap;

		Other.Str = nullptr;
		Other.Len = 0;
		Other.Cap = 0;
	}

	template<typename T, typename Alloca>
	template<typename Iter>
	BasicString<T, Alloca>::BasicString(Iter Begin, Iter End)
	{
		SizeType NewLen = End - Begin;
		CreateRawString(NewLen);

		std::memcpy(
			Str,
			std::addressof(*Begin),
			NewLen * sizeof(ValueType)
		);
	}

	template<typename T, typename Alloca>
	BasicString<T, Alloca>& BasicString<T, Alloca>::operator=(const BasicString &Other)
	{
		if (this == &Other) return *this;

		if (Cap >= Other.Len + 1)
			StrCopy(Str, Other.Str, Other.Len);
		else
		{
			if (Str) Alloc.Deallocate(Str);
			Str = Alloc.Allocate(Other.Len + 1);
			StrCopy(Str, Other.Str, Other.Len);
			Cap = Other.Len + 1;
		}

		Len = Other.Len;
		return *this;
	}

	template<typename T, typename Alloca>
	BasicString<T, Alloca> & BasicString<T, Alloca>::operator=(BasicString &&Other) noexcept
	{
		if (this != Other)
		{
			Str = Other.Str;
			Cap = Other.Cap;
			Len = Other.Len;

			Str = nullptr;
			Cap = 0;
			Len = 0;
		}

		return *this;
	}

	template<typename T, typename Alloca>
	void BasicString<T, Alloca>::Reserve(SizeType NewCap)
	{
		if (NewCap <= Cap)
			return;

		ValueType* NewStr = Alloc.Allocate(NewCap);

		if (Str)
		{
			StrCopy(NewStr, Str, Len);
			Alloc.Deallocate(Str);
		}

		Str = NewStr;
		Cap = NewCap;
	}

	template<typename T, typename Alloca>
	void BasicString<T, Alloca>::Add(T Ch)
	{
		RawResize(Len + 1);
		Str[Len - 1] = Ch;
	}

	template<typename T, typename Alloca>
	void BasicString<T, Alloca>::Append(const BasicString &Other)
	{
		SizeType OldLen = Len;
		RawResize(Len + Other.Len);
		std::memcpy(Str + OldLen, Other.Str, Other.Len * sizeof(ValueType));
	}

	template<typename T, typename Alloca>
	BasicString<T, Alloca> BasicString<T, Alloca>::SubStr(RelativeStringRef Ref)
	{
		assert(Ref.Pos <= Len);
		assert(Ref.Pos + Ref.Length <= Len);

		auto It = Begin() + Ref.Pos;
		return BasicString(It, It + Ref.Length);
	}

	template<typename T, typename Alloca>
	void BasicString<T, Alloca>::CreateRawString(SizeType InLen)
	{
		Cap = CalculateCapacity(InLen + 1);
		Str = Alloc.Allocate(Cap);
		Str[InLen] = T(0);
		Len = InLen;
	}

	template<typename T, typename Alloca>
	void BasicString<T, Alloca>::RawResize(SizeType NewLen)
	{
		if (NewLen == Len) return;

		if (NewLen > Cap)
			Reserve(CalculateCapacity(NewLen + 1));

		Len = NewLen;
		Str[Len] = T(0);
	}

	template<typename T, typename Alloca>
	BasicString<T, Alloca>::SizeType BasicString<T, Alloca>::StrLength(const T *Str)
	{
		SizeType Len = 0;
		while (Str[Len] != T(0))
			++Len;

		return Len;
	}

	template<typename T, typename Alloca>
	void BasicString<T, Alloca>::StrCopy(ValueType *Dst, const ValueType *Src, size_t Len)
	{
		std::memcpy(Dst, Src, (Len + 1) * sizeof(ValueType));
	}

	using String = BasicString<char>;
	using UString = BasicString<char32_t>;

	inline std::ostream& operator<<(std::ostream& Os, const String& Str)
	{
		return Os << Str.CStr();
	}
}

#endif //CVOLT_STRING_H