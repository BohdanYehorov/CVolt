//
// Created by bohdan on 22.01.26.
//

#ifndef CVOLT_TYPEDEFS_H
#define CVOLT_TYPEDEFS_H

#include <unordered_map>
#include <string>

namespace Volt
{
	template <typename T>
	using SmallVec8 = llvm::SmallVector<T, 8>;

	template <typename T>
	using SmallVec16 = llvm::SmallVector<T, 16>;

	template <typename T>
	using SmallVec32 = llvm::SmallVector<T, 32>;

	template <typename T>
	using SmallVec64 = llvm::SmallVector<T, 64>;
}

#endif //CVOLT_TYPEDEFS_H