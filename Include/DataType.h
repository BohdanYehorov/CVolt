//
// Created by bohdan on 22.12.25.
//

#ifndef CVOLT_DATATYPE_H
#define CVOLT_DATATYPE_H
#include "TypeDefs.h"

namespace Volt
{
    enum class DataType : UInt8
    {
        VOID,
        BOOL, CHAR,
        BYTE, INT, LONG,
        FLOAT, DOUBLE
    };
}

#endif //CVOLT_DATATYPE_H