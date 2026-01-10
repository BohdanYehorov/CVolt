//
// Created by bohdan on 22.12.25.
//

#ifndef CVOLT_PRIMITIVE_DATAT_YPE_H
#define CVOLT_PRIMITIVE_DATAT_YPE_H
#include "TypeDefs.h"

namespace Volt
{
    enum class PrimitiveDataType : UInt8
    {
        VOID,
        BOOL, CHAR,
        BYTE, INT, LONG,
        FLOAT, DOUBLE
    };

    enum class IntegerType
    {
        CHAR, BYTE, INT, LONG
    };

    enum class FPType
    {
        FLOAT, DOUBLE
    };
}

#endif //CVOLT_PRIMITIVE_DATAT_YPE_H