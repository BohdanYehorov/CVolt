//
// Created by bohdan on 7/30/26.
//

#ifndef CVOLT_FUNCTIONOVERLOAD_H
#define CVOLT_FUNCTIONOVERLOAD_H

#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Core/Types/DataType.h"
#include "BuiltinFuncCallee.h"

namespace Volt
{
    template <typename CalleeType>
    struct FuncOverloadImpl
    {
        ArgsVector<QualType> Args;
        CalleeType* Callee;

        FuncOverloadImpl(ArgsVector<QualType> Args, CalleeType* Callee)
            : Args(std::move(Args)), Callee(Callee) {}
    };

    using FunctionOverload        = FuncOverloadImpl<FunctionCallee>;
    using MethodOverload          = FuncOverloadImpl<MethodCallee>;
    using BuiltinFunctionOverload = FuncOverloadImpl<BuiltinFuncCallee>;
}
#endif //CVOLT_FUNCTIONOVERLOAD_H
