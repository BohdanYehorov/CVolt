//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_FUNCTIONSIGNATURE_H
#define CVOLT_FUNCTIONSIGNATURE_H

#include <llvm/ADT/SmallVector.h>
#include "Volt/Compiler/Types/DataType.h"
#include <string>

namespace Volt
{
    struct FunctionSignature
    {
        std::string Name;
        llvm::SmallVector<DataTypeBase*, 8> Params;

        FunctionSignature() = default;
        FunctionSignature(const std::string& Name, llvm::ArrayRef<DataTypeBase*> Params)
                : Name(Name), Params(Params) {}

        [[nodiscard]] bool operator==(const FunctionSignature& Other) const
        {
            if (Name != Other.Name || Params.size() != Other.Params.size())
                return false;

            for (size_t i = 0; i < Params.size(); i++)
                // if (Params[i] != Other.Params[i])
                if (!DataTypeUtils::IsEqual(Params[i], Other.Params[i]))
                    return false;

            return true;
        }
    };
}

#endif //CVOLT_FUNCTIONSIGNATURE_H