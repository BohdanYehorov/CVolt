//
// Created by bohdan on 7/30/26.
//

#ifndef CVOLT_FUNCTIONOVERLOAD_H
#define CVOLT_FUNCTIONOVERLOAD_H

#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Core/Types/DataType.h"
#include "Callee.h"

namespace Volt
{
    struct FunctionOverload
    {
        ArgsVector<QualType> Args;
        CalleeBase* Callee;

        FunctionOverload(ArgsVector<QualType> Args, CalleeBase* Callee)
            : Args(std::move(Args)), Callee(Callee) {}
    };
}
#endif //CVOLT_FUNCTIONOVERLOAD_H
