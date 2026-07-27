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
		Array(SizeType Len, const ValueType& Fill = ValueType()) { Resize(Len, Fill); }
		Array(const std::initializer_list<ValueType>& List)
		{
			static_assert(std::is_copy_constructible_v<ValueType>);
			RawResize(List.size());
			CopyRange(Data, List.begin(), Len);
		}

		Array(const Array& Other)
		{
			static_assert(std::is_copy_constructible_v<ValueType>);
			Len = Other.Len;
			Cap = Other.Cap;
			Data = Alloc.Allocate(Cap);
			CopyRange(Data, Other.Data, Len);
		}

		Array(Array&& Other) noexcept
		{
			std::swap(Len, Other.Len);
			std::swap(Cap, Other.Cap);
			std::swap(Data, Other.Data);
		}

		~Array() { Deallocate(); }

		std::enable_if_t<std::is_copy_constructible_v<ValueType>, Array&>
		operator=(const Array& Other) noexcept
		{
			if (this != &Other)
			{
				ValueType* NewData = Alloc.Allocate(Other.Cap);
				CopyRange(NewData, Other.Data, Other.Len);
				if (Data) Deallocate();
				Data = NewData;
				Len = Other.Len;
				Cap = Other.Cap;
			}

			return *this;
		}

		Array& operator=(Array&& Other) noexcept
		{
			if (this != &Other)
			{
				if (Data)
					Deallocate();
				Len = Other.Len;
				Cap = Other.Cap;
				Data = Other.Data;
				Other.Reset();
			}

			return *this;
		}

		[[nodiscard]] ValueType& operator[](SizeType Index) { return Data[Index]; }
		[[nodiscard]] const ValueType& operator[](SizeType Index) const { return Data[Index]; }

		[[nodiscard]] SizeType Length() const { return Len; }
		[[nodiscard]] SizeType Capacity() const { return Cap; }
		[[nodiscard]] ValueType* RawData() { return Data; }
		[[nodiscard]] const ValueType* RawData() const { return Data; }

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

		void Reserve(SizeType NewCap)
		{
			if (NewCap <= Cap) return;
			ValueType* NewData = Alloc.Allocate(NewCap);
			if (Data)
			{
				TransferRange(NewData, Data, Len);
				DestructRange(Data, Len);
				Alloc.Deallocate(Data);
			}
			Data = NewData;
			Cap = NewCap;
		}

		void Add(const ValueType& Value)
		{
			if (Len >= Cap) Reserve(CalculateCapacity(Cap));
			Copy(Data + Len, Value);
			Len++;
		}

		void Add(ValueType&& Value)
		{
			if (Len >= Cap) Reserve(CalculateCapacity(Cap));
			Move(Data + Len, std::move(Value));
			Len++;
		}

		template <typename ...Args_>
		void Emplace(Args_&&... Args)
		{
			if (Len >= Cap) Reserve(CalculateCapacity(Cap));
			Alloc.Construct(Data + Len, std::forward<Args_>(Args)...);
			Len++;
		}

		void Pop()
		{
			if (Len == 0) return;
			Destruct(Data + (Len - 1));
			Len--;
		}

		void Resize(SizeType NewLen, const ValueType& Fill = ValueType())
		{
			SizeType OldLen = Len;
			RawResize(NewLen);
			if (Len > OldLen)
				for (SizeType i = OldLen; i < Len; i++)
					Copy(Data + i, Fill);
		}

		void Clear() { RawResize(0); }

		void ShrinkToFit()
		{
			if (Len == Cap) return;
			if (Len == 0)
			{
				Deallocate();
				return;
			}

			ValueType* NewData = Alloc.Allocate(Len);
			TransferRange(NewData, Data, Len);
			DestructRange(Data, Len);
			Alloc.Deallocate(Data);
			Data = NewData;
			Cap = Len;
		}

	private:
		static SizeType CalculateCapacity(SizeType InCap)
		{
			if (InCap < 32) return 32;
			return SizeType(1) << (static_cast<SizeType>(std::bit_width(InCap)));
		}

		void RawResize(SizeType NewLen)
		{
			if (Len == NewLen) return;
			if (Len > NewLen)
				DestructRange(Data + NewLen, Len - NewLen);
			else if (Cap < NewLen)
				Reserve(CalculateCapacity(NewLen));
			Len = NewLen;
		}

		void Reset()
		{
			Len = Cap = 0;
			Data = nullptr;
		}

		void Deallocate()
		{
			if (!Data) return;

			DestructRange(Data, Len);
			Alloc.Deallocate(Data);
			Reset();
		}

		void Copy(ValueType* Dst, const ValueType& Src)
		{
			if constexpr (std::is_trivially_copyable_v<ValueType>)
			{
				*Dst = Src;
				return;
			}
			Alloc.Construct(Dst, Src);
		}

		void Move(ValueType* Dst, ValueType&& Src)
		{
			if constexpr (std::is_trivially_copyable_v<ValueType>)
			{
				*Dst = std::move(Src);
				return;
			}
			Alloc.Construct(Dst, std::move(Src));
		}

		void Transfer(ValueType* Dst, ValueType& Src)
		{
			if constexpr (std::is_trivially_copyable_v<ValueType>)
				std::memcpy(Dst, &Src, sizeof(ValueType));
			else if constexpr (std::is_nothrow_move_constructible_v<ValueType>)
				Alloc.Construct(Dst, std::move(Src));
			else
				Alloc.Construct(Dst, Src);
		}

		void Destruct(ValueType* Ptr)
		{
			if constexpr (std::is_trivially_destructible_v<ValueType>) return;
			Alloc.Destruct(Ptr);
		}

		void CopyRange(ValueType* Dst, const ValueType* Src, SizeType Len)
		{
			if (Len == 0) return;
			if constexpr (std::is_trivially_copyable_v<ValueType>)
			{
				std::memcpy(Dst, Src, Len * sizeof(ValueType));
				return;
			}

			for (SizeType i = 0; i < Len; i++)
				Alloc.Construct(Dst + i, Src[i]);
		}

		void TransferRange(ValueType* Dst, ValueType* Src, SizeType Len)
		{
			if (Len == 0) return;
			if constexpr (std::is_trivially_copyable_v<ValueType>)
				std::memcpy(Dst, Src, Len * sizeof(ValueType));
			else if constexpr (std::is_nothrow_move_constructible_v<ValueType>)
				for (SizeType i = 0; i < Len; i++)
					Alloc.Construct(Dst + i, std::move(Src[i]));
			else
				for (SizeType i = 0; i < Len; i++)
					Alloc.Construct(Dst + i, Src[i]);
		}

		void DestructRange(ValueType* Ptr, SizeType Len)
		{
			if constexpr (std::is_trivially_destructible_v<ValueType>) return;

			while (Len > 0)
			{
				--Len;
				Alloc.Destruct(Ptr + Len);
			}
		}
	};
}
#endif //CVOLT_ARRAY_H