//
// Created by bohdan on 7/30/26.
//

#ifndef CVOLT_FUNCTIONOVERLOAD_H
#define CVOLT_FUNCTIONOVERLOAD_H

#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Core/Types/DataType.h"
#include "BuiltinFuncCallee.h"
#include "Callee.h"

namespace Volt
{
    struct OverloadBase
    {
        ArgsVector<QualType> Args;

        OverloadBase(ArgsVector<QualType> Args)
            : Args(std::move(Args)) {}
    };

    struct FunctionOverload : OverloadBase
    {
        CalleeBase* Callee;

        FunctionOverload(ArgsVector<QualType> Args, CalleeBase* Callee)
            : OverloadBase(std::move(Args)), Callee(Callee) {}
    };

    struct MethodOverload : FunctionOverload
    {
        class ClassType* ThisType;

        MethodOverload(ArgsVector<QualType> Args, CalleeBase* Callee, ClassType* ThisType)
            : FunctionOverload(std::move(Args), Callee), ThisType(ThisType) {}
    };

    struct BuiltinFunctionOverload : OverloadBase
    {
        BuiltinFuncCallee* Callee;

        BuiltinFunctionOverload(ArgsVector<QualType> Args, BuiltinFuncCallee* Callee)
            : OverloadBase(std::move(Args)), Callee(Callee) {}
    };
}
#endif //CVOLT_FUNCTIONOVERLOAD_H
