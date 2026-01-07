//
// Created by bohdan on 22.12.25.
//

#ifndef CVOLT_DATATYPE_H
#define CVOLT_DATATYPE_H
#include "TypeDefs.h"

enum class DataType : UInt8
{
    VOID,
    BOOL, CHAR,
    BYTE, INT, LONG,
    FLOAT, DOUBLE
};

struct DataTypeInfo
{
    DataType BaseType;
    size_t Size;
    size_t Align;
};

#endif //CVOLT_DATATYPE_H