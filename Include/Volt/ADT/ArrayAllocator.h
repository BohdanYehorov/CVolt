//
// Created by bohdan on 08.02.26.
//

#ifndef CVOLT_ARRAYALLOCATOR_H
#define CVOLT_ARRAYALLOCATOR_H

#include <cstddef>
#include <utility>

namespace Volt
{
	template <typename T>
	class ArrayAllocator
	{
	public:
		T* Allocate(size_t Count);
		void Deallocate(T* Data);

		template <typename ... Args_>
		void Construct(T* Ptr, Args_&& ... Args);

		void Destruct(T* Ptr);
	};

	template<typename T>
	T *ArrayAllocator<T>::Allocate(size_t Count)
	{
		return static_cast<T*>(::operator new(Count * sizeof(T)));
	}

	template<typename T>
	void ArrayAllocator<T>::Deallocate(T *Data)
	{
		::operator delete(Data);
	}

	template<typename T>
	template<typename ... Args_>
	void ArrayAllocator<T>::Construct(T *Ptr, Args_&&... Args)
	{
		new (Ptr) T(std::forward<Args_>(Args)...);
	}

	template<typename T>
	void ArrayAllocator<T>::Destruct(T *Ptr)
	{
		Ptr->~T();
	}
}

#endif //CVOLT_ARRAYALLOCATOR_H