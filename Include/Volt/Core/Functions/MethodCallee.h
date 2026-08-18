//
// Created by bohdan on 8/18/26.
//

#ifndef CVOLT_METHODCALLEE_H
#define CVOLT_METHODCALLEE_H

#include "FunctionCallee.h"

namespace Volt
{
    class MethodCallee : public FunctionCallee
    {
        GENERATED_BODY(MethodCallee, FunctionCallee)
    public:
        class ClassType* Owner;

        MethodCallee(QualType Type, llvm::Function* Function, ClassType* Owner)
            : FunctionCallee(Type, Function), Owner(Owner) {}
    };
}

#endif //CVOLT_METHODCALLEE_H
