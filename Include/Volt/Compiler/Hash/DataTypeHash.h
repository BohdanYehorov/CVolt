//
// Created by bohdan on 13.01.26.
//

#ifndef CVOLT_DATATYPEHASH_H
#define CVOLT_DATATYPEHASH_H
#include "Volt/Compiler/Types/DataTypeBase.h"

namespace Volt
{
    class DataTypeHash
    {
    public:
        size_t operator()(const DataTypeBase* Type) const;
    };
}

#endif //CVOLT_DATATYPEHASH_H