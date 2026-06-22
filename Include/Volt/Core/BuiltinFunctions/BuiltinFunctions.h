//
// Created by bohdan on 21.01.26.
//

#ifndef CVOLT_BUILTINFUNCTIONS_H
#define CVOLT_BUILTINFUNCTIONS_H

#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include <iostream>
#include <cmath>
#include <ctime>
#include <random>
#include <cstring>

extern "C"
{
	inline void OutBool(bool B)
	{
		std::cout << std::boolalpha << B << std::endl;
	}

	inline void OutChar(char Ch)
	{
		std::cout << Ch << std::endl;
	}

	inline void OutI8(Volt::Int8 Num)
	{
		std::cout << static_cast<int>(Num) << std::endl;
	}

	inline void OutI16(Volt::Int16 Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutI32(Volt::Int32 Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutI64(Volt::Int64 Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutU8(Volt::UInt8 Num)
	{
		std::cout << static_cast<Volt::UInt32>(Num) << std::endl;
	}

	inline void OutU16(Volt::UInt16 Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutU32(Volt::UInt32 Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutU64(Volt::UInt64 Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutStr(const char* Str)
	{
		std::cout << Str << std::endl;
	}

	inline void OutFloat(float Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutDouble(double Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutPtr(void* Ptr)
	{
		std::cout << Ptr << std::endl;
	}

	inline void InInt(int& Num)
	{
		std::cin >> Num;
	}

	inline void InIntWithLabel(char* Label, int& Num)
	{
		std::cout << Label << " ";
		std::cin >> Num;
	}

	inline long Time()
	{
		return std::time(nullptr);
	}

	inline double Sqrt(double X)
	{
		return std::sqrt(X);
	}

	inline double Sin(double Angle)
	{
		return std::sin(Angle);
	}

	inline double Cos(double Andle)
	{
		return std::cos(Andle);
	}

	inline double Tan(double Angle)
	{
		return std::tan(Angle);
	}

	inline int RandomInt(int Min, int Max)
	{
		static std::mt19937 Gen{ std::random_device{}() };
		std::uniform_int_distribution<int> Dist(Min, Max);
		return Dist(Gen);
	}

	inline int System(char* Cmd)
	{
		return system(Cmd);
	}

	inline void* MemAlloc(long Size)
	{
		return std::malloc(Size);
	}

	inline void MemFree(void* Data)
	{
		std::free(Data);
	}

	inline void* Realloc(void* Data, long NewSize)
	{
		return std::realloc(Data, NewSize);
	}

	inline void MemCpy(void* Dst, void* Src, long Size)
	{
		std::memcpy(Dst, Src, Size);
	}
}

#endif //CVOLT_BUILTINFUNCTIONS_H