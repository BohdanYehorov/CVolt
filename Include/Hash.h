//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_HASH_H
#define CVOLT_HASH_H

#include "ASTNodes.h"
#include "FunctionSignature.h"

namespace Volt
{
    size_t CombineHashes(llvm::ArrayRef<size_t> Hashes);

    inline void CombineHashes(size_t& Seed, size_t Hash)
    {
        Seed ^= Hash + 0x9e3779b9 + (Seed << 6) + (Seed >> 2);
    }

    class DataTypeHash
    {
    public:
        size_t operator()(const DataTypeNodeBase* Type) const;
    };

    class FunctionSignatureHash
    {
    public:
        size_t operator()(const FunctionSignature& FuncSign) const;
    };
}

#endif //CVOLT_HASH_H