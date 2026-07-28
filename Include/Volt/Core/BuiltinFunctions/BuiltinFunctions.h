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

template <typename T>
void Out(T Value)
{
	llvm::outs() << Value;
}

template <>
inline void Out<bool>(bool Value)
{
	static const char* Outputs[]{ "false", "true" };
	llvm::outs() << Outputs[Value];
}

template <>
inline void Out<Volt::Int8>(Volt::Int8 Value)
{
	llvm::outs() << static_cast<int>(Value);
}

template <>
inline void Out<Volt::UInt8>(Volt::UInt8 Value)
{
	llvm::outs() << static_cast<int>(Value);
}

template <typename T>
void OutLine(T Value)
{
	Out(Value);
	llvm::outs() << '\n';
}

template <typename T>
void In(T& Num)
{
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

inline int RandomInt(Volt::Int64 Min, Volt::Int64 Max)
{
	static std::mt19937 Gen{ std::random_device{}() };
	std::uniform_int_distribution<Volt::Int64> Dist(Min, Max);
	return Dist(Gen);
}

inline Volt::Int32 System(char* Cmd)
{
	return system(Cmd);
}

inline void* MemAlloc(Volt::UInt64 Size)
{
	return std::malloc(Size);
}

inline void MemFree(void* Data)
{
	std::free(Data);
}

inline void* Realloc(void* Data, Volt::UInt64 NewSize)
{
	return std::realloc(Data, NewSize);
}

inline void MemCpy(void* Dst, void* Src, Volt::UInt64 Size)
{
	std::memcpy(Dst, Src, Size);
}

#endif //CVOLT_BUILTINFUNCTIONS_H