//
// Created by bohdan on 7/30/26.
//

#ifndef CVOLT_FUNCTIONOVERLOAD_H
#define CVOLT_FUNCTIONOVERLOAD_H

#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Core/Types/DataType.h"
#include "FunctionCallee.h"
#include "MethodCallee.h"
#include "BuiltinFuncCallee.h"
#include "Volt/ADT/Array.h"

namespace Volt
{
    template <typename CalleeType>
    struct FuncOverloadImpl
    {
        ArgsVector<QualType> Args;
        CalleeType* Callee;

        FuncOverloadImpl(ArgsVector<QualType> Args, CalleeType* Callee)
            : Args(std::move(Args)), Callee(Callee) {}

        [[nodiscard]] bool GetCastKindsAndCheckIsValidCasts(
            llvm::ArrayRef<QualType> TargetArgs, Array<CastKind>& Kinds) const
        {
            if (Args.size() != TargetArgs.size()) return false;
            Kinds.Reserve(Args.size());

            for (size_t i = 0; i < Args.size(); i++)
            {
                if (auto* RefType = Args[i].CastAs<ReferenceType>())
                {
                    if (RefType->CanBind(TargetArgs[i]))
                    {
                        Kinds.Add(CastKind::Exact);
                        continue;
                    }
                }

                CastKind Kind = TargetArgs[i].CastTo(Args[i]);
                if (!DataType::IsImplicitCastKind(Kind))
                    return false;

                Kinds.Add(Kind);
            }

            return true;
        }
    };

    using FunctionOverload        = FuncOverloadImpl<FunctionCallee>;
    using MethodOverload          = FuncOverloadImpl<MethodCallee>;
    using BuiltinFunctionOverload = FuncOverloadImpl<BuiltinFuncCallee>;
}
#endif //CVOLT_FUNCTIONOVERLOAD_H
