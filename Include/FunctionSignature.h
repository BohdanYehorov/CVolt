//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_FUNCTIONSIGNATURE_H
#define CVOLT_FUNCTIONSIGNATURE_H

#include <llvm/ADT/SmallVector.h>
#include "ASTNodes.h"
#include <string>

namespace Volt
{
    struct FunctionSignature
    {
        std::string Name;
        DataTypeNodeBase* ReturnType;
        llvm::SmallVector<DataTypeNodeBase*, 8> Params;

        FunctionSignature(const std::string& Name, DataTypeNodeBase* ReturnType, llvm::ArrayRef<DataTypeNodeBase*> Params)
            : Name(Name), ReturnType(ReturnType), Params(Params) {}
    };
}

#endif //CVOLT_FUNCTIONSIGNATURE_H