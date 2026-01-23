//
// Created by bohdan on 21.01.26.
//

#ifndef CVOLT_BUILTINFUNCTIONS_H
#define CVOLT_BUILTINFUNCTIONS_H

#include <iostream>
#include <cmath>
#include <ctime>
#include <random>

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

	inline void OutByte(std::byte Byte)
	{
		std::cout << static_cast<int>(Byte) << std::endl;
	}

	inline void OutInt(int Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutLong(long Num)
	{
		std::cout << Num << std::endl;
	}

	inline void OutStr(char* Str)
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

	inline void InInt(int* Num)
	{
		std::cin >> *Num;
	}

	inline void InIntWithLabel(char* Label, int* Num)
	{
		std::cout << Label << " ";
		std::cin >> *Num;
	}

	inline long Time()
	{
		return std::time(nullptr);
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
}

#endif //CVOLT_BUILTINFUNCTIONS_H