//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_HASH_H
#define CVOLT_HASH_H

#include <hash_fun.h>
#include "ASTNodes.h"

class DataTypeHash
{
    size_t operator()(const DataTypeNodeBase* Type) const;
};


#endif //CVOLT_HASH_H