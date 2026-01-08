//
// Created by bohdan on 08.01.26.
//

#ifndef CVOLT_TYPEDVALUE_H
#define CVOLT_TYPEDVALUE_H
#include <llvm/IR/Value.h>
#include "ASTNodes.h"

namespace Volt
{
    class TypedValue
    {
    private:
        llvm::Value* Value = nullptr;
        DataTypeNodeBase* Type = nullptr;

    public:
        TypedValue() = default;
        TypedValue(llvm::Value* Value, DataTypeNodeBase* Type)
            : Value(Value), Type(Type) {}

        [[nodiscard]] llvm::Value* GetValue() const { return Value; }
        [[nodiscard]] DataTypeNodeBase* GetType() const { return Type; }
    };
}


#endif //CVOLT_TYPEDVALUE_H