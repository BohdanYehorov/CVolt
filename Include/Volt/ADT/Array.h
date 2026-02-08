//
// Created by bohdan on 08.02.26.
//

#ifndef CVOLT_ARRAY_H
#define CVOLT_ARRAY_H

#include "ArrayAllocator.h"
#include "ArrayIterator.h"
#include <initializer_list>

namespace Volt
{
	template <typename T, typename Alloca = ArrayAllocator<T>>
	class Array
	{
	public:
		using ValueType = T;
		using SizeType = size_t;
		using DifferenceType = std::ptrdiff_t;
		using AllocatorType = Alloca;
		using Iterator = ArrayIterator<T>;
		using ConstIterator = ArrayIterator<const T>;

	private:
		T* Data = nullptr;
		SizeType Len = 0;
		SizeType Cap = 0;
		AllocatorType Alloc;

	public:
		Array() = default;
		Array(const std::initializer_list<T>& List);
		Array(SizeType Len, const T& Value);

		Array(const Array& Other);
		Array(Array&& Other) noexcept;

		Array& operator=(const Array& Other);
		Array& operator=(Array&& Other) noexcept;

		~Array() { Deallocate(); }

		void Reserve(SizeType NewCap);
		void Resize(SizeType NewLen, const ValueType& FillValue);

		void Add(const T& Value);
		void Pop();

		template <typename ... Args_>
		void Emplace(Args_ ... Args);

		[[nodiscard]] const T& operator[](SizeType Index) const { return Data[Index]; }
		[[nodiscard]] T& operator[](SizeType Index) { return Data[Index]; }

		[[nodiscard]] SizeType Length() const { return Len; }
		[[nodiscard]] SizeType Capacity() const { return Cap; }
		[[nodiscard]] bool Empty() const { return Len == 0; }

		[[nodiscard]] ValueType& Front() { return Data[0]; }
		[[nodiscard]] const ValueType& Front() const { return Data[0]; }

		[[nodiscard]] ValueType& Back() { return Data[Len - 1]; }
		[[nodiscard]] const ValueType& Back() const { return Data[Len - 1]; }

		[[nodiscard]] Iterator Begin() { return Iterator(Data); }
		[[nodiscard]] Iterator End() { return Iterator(Data + Len); }

		[[nodiscard]] Iterator begin() { return Iterator(Data); }
		[[nodiscard]] Iterator end() { return Iterator(Data + Len); }

		[[nodiscard]] ConstIterator begin() const { return ConstIterator(Data); }
		[[nodiscard]] ConstIterator end()   const { return ConstIterator(Data + Len); }

	private:
		void RawResize(SizeType NewLen);
		void Deallocate();

		static SizeType CalculateCapacity(SizeType InSize) { return InSize * 2; }
	};

	template<typename T, typename Alloca>
	Array<T, Alloca>::Array(const std::initializer_list<T> &List)
		: Len(List.size()), Cap(CalculateCapacity(Len))
	{
		Data = Alloc.Allocate(Cap);
		SizeType i = 0;

		for (const T& El : List)
		{
			Alloc.Construct(Data + i, El);
			i++;
		}
	}

	template<typename T, typename Alloca>
	Array<T, Alloca>::Array(SizeType Len, const T &Value)
		: Len(Len), Cap(CalculateCapacity(Len))
	{
		Data = Alloc.Allocate(Cap);

		for (SizeType i = 0; i < Len; i++)
			Alloc.Construct(Data + i, Value);
	}

	template<typename T, typename Alloca>
	Array<T, Alloca>::Array(const Array &Other)
	{
		if (Other.Len == 0)
			return;

		Data = Alloc.Allocate(Other.Cap);
		for (SizeType i = 0; i < Other.Len; i++)
			Alloc.Construct(Data + i, Other.Data[i]);

		Len = Other.Len;
		Cap = Other.Cap;
	}

	template<typename T, typename Alloca>
	Array<T, Alloca>::Array(Array &&Other) noexcept
	{
		Data = Other.Data;
		Len = Other.Len;
		Cap = Other.Cap;

		Other.Data = nullptr;
		Other.Len = 0;
		Other.Cap = 0;
	}

	template<typename T, typename Alloca>
	Array<T, Alloca> &Array<T, Alloca>::operator=(const Array &Other)
	{
		if (this != &Other)
		{
			if (Data)
				Deallocate(Data);

			Data = Alloc.Allocate(Other.Cap);
			for (SizeType i = 0; i < Other.Len; i++)
				Alloc.Construct(Data + i, Other.Data[i]);

			Cap = Other.Cap;
			Len = Other.Len;
		}

		return *this;
	}

	template<typename T, typename Alloca>
	Array<T, Alloca> &Array<T, Alloca>::operator=(Array &&Other) noexcept
	{
		if (this != &Other)
		{
			if (Data)
				Deallocate(Data);

			Data = Other.Data;
			Cap = Other.Cap;
			Len = Other.Len;

			Other.Data = nullptr;
			Other.Cap = Cap;
			Other.Len = Len;
		}

		return *this;
	}

	template<typename T, typename Alloca>
	void Array<T, Alloca>::Reserve(SizeType NewCap)
	{
		if (NewCap <= Cap) return;

		ValueType* NewData = Alloc.Allocate(NewCap);
		for (SizeType i = 0; i < Len; i++)
		{
			Alloc.Construct(NewData + i, std::move(Data[i]));
			Alloc.Destruct(Data + i);
		}

		Alloc.Deallocate(Data);
		Data = NewData;
		Cap = NewCap;
	}

	template<typename T, typename Alloca>
	void Array<T, Alloca>::Resize(SizeType NewLen, const ValueType &FillValue)
	{
		if (NewLen == Len) return;

		if (NewLen < Len)
		{
			for (SizeType i = NewLen; i < Len; i++)
				Alloc.Destruct(Data + i);

			Len = NewLen;
			return;
		}

		if (NewLen > Cap)
			Reserve(CalculateCapacity(NewLen));

		for (SizeType i = Len; i < NewLen; i++)
			Alloc.Construct(Data + i, FillValue);

		Len = NewLen;
	}

	template<typename T, typename Alloca>
	void Array<T, Alloca>::Add(const T &Value)
	{
		if (Len + 1 > Cap)
			Reserve(CalculateCapacity(Len + 1));

		Alloc.Construct(Data + Len, Value);
		Len++;
	}

	template<typename T, typename Alloca>
	void Array<T, Alloca>::Pop()
	{
		if (Len == 0) return;

		Alloc.Destruct(Data + (Len - 1));
		Len--;
	}

	template<typename T, typename Alloca>
	template<typename ... Args_>
	void Array<T, Alloca>::Emplace(Args_... Args)
	{
		if (Len + 1 > Cap)
			Reserve(CalculateCapacity(Len + 1));

		Alloc.Construct(Data + Len, std::forward<Args_>(Args)...);
		Len++;
	}

	template<typename T, typename Alloca>
	void Array<T, Alloca>::RawResize(SizeType NewLen)
	{
		if (NewLen == Len) return;

		if (NewLen < Len)
		{
			for (SizeType i = NewLen; i < Len; i++)
				Alloc.Destruct(Data + i);

			Len = NewLen;
			return;
		}

		if (NewLen > Cap)
			Reserve(CalculateCapacity(NewLen));

		Len = NewLen;
	}

	template<typename T, typename Alloca>
	void Array<T, Alloca>::Deallocate()
	{
		if (!Data) return;

		for (SizeType i = 0; i < Len; i++)
			Alloc.Destruct(Data + i);

		Alloc.Deallocate(Data);

		Data = nullptr;
		Len = 0;
		Cap = 0;
	}
}

#endif //CVOLT_ARRAY_H