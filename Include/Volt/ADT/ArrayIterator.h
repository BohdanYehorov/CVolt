//
// Created by bohdan on 08.02.26.
//

#ifndef CVOLT_ARRAYITERATOR_H
#define CVOLT_ARRAYITERATOR_H

#include <cstddef>

namespace Volt
{
	template <typename T>
	class ArrayIterator
	{
	public:
		using ValueType = T;
		using PointerType = T*;
		using ConstPointerType = const T*;
		using DifferenceType = std::ptrdiff_t;

	private:
		PointerType Ptr = nullptr;

	public:
		ArrayIterator(PointerType Ptr) : Ptr(Ptr) {}

		[[nodiscard]] PointerType Get() { return Ptr; }
		[[nodiscard]] ConstPointerType Get() const { return Ptr; }

		[[nodiscard]] ValueType& operator*() { return *Ptr; }
		[[nodiscard]] const ValueType& operator*() const { return *Ptr; }

		[[nodiscard]] PointerType operator->() { return Ptr; }
		[[nodiscard]] ConstPointerType operator->() const { return Ptr; }

		ArrayIterator& operator+=(DifferenceType N)
		{
			Ptr += N;
			return *this;
		}

		ArrayIterator& operator-=(DifferenceType N)
		{
			Ptr -= N;
			return *this;
		}

		ArrayIterator& operator++()
		{
			++Ptr;
			return *this;
		}

		ArrayIterator& operator--()
		{
			--Ptr;
			return *this;
		}

		ArrayIterator operator++(int)
		{
			ArrayIterator Temp = *this;
			++(*this);
			return Temp;
		}

		ArrayIterator operator--(int)
		{
			ArrayIterator Temp = *this;
			--(*this);
			return Temp;
		}

		[[nodiscard]] bool operator==(const ArrayIterator &Other) const
		{
			return Ptr == Other.Ptr;
		}

		[[nodiscard]] bool operator!=(const ArrayIterator &Other) const
		{
			return Ptr != Other.Ptr;
		}

		friend ArrayIterator<T> operator+(ArrayIterator Left, DifferenceType Right)
		{
			return ArrayIterator(Left.Ptr + Right);
		}

		friend ArrayIterator<T> operator+(DifferenceType Left, ArrayIterator Right)
		{
			return ArrayIterator(Left + Right.Ptr);
		}

		friend ArrayIterator<T> operator-(ArrayIterator Left, DifferenceType Right)
		{
			return ArrayIterator(Left.Ptr - Right);
		}

		friend DifferenceType operator-(ArrayIterator Left, ArrayIterator Right)
		{
			return Left.Ptr - Right.Ptr;
		}
	};
}

#endif //CVOLT_ARRAYITERATOR_H