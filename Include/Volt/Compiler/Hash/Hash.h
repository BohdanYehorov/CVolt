//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_HASH_H
#define CVOLT_HASH_H

#include <llvm/ADT/SmallVector.h>

namespace Volt
{
    class FunctionSignature;

    size_t CombineHashes(llvm::ArrayRef<size_t> Hashes);

    inline void CombineHashes(size_t& Seed, size_t Hash)
    {
        Seed ^= Hash + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
    }
}

#endif //CVOLT_HASH_H