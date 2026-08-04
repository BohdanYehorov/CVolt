//
// Created by bohdan on 8/4/26.
//

#ifndef CVOLT_FUNCOVERLOADTABLE_H
#define CVOLT_FUNCOVERLOADTABLE_H

#include "Volt/Core/TypeDefs/FunctionDefs.h"

namespace Volt
{
    class FuncOverloadTable
    {
    public:
        using Iterator = FuncOverloadVector::iterator;
        using ConstIterator = FuncOverloadVector::const_iterator;

    private:
        FuncOverloadVector Overloads;

    public:
        void AddOverload(ArgsVector<QualType> Args, CalleeBase* Callee)
        {
            Overloads.emplace_back(std::move(Args), Callee);
        }

        [[nodiscard]] const FunctionOverload* FindBestOverload(llvm::ArrayRef<QualType> Args) const;

        [[nodiscard]] Iterator begin() { return Overloads.begin(); }
        [[nodiscard]] Iterator end() { return Overloads.end(); }
        [[nodiscard]] ConstIterator begin() const { return Overloads.begin(); }
        [[nodiscard]] ConstIterator end() const { return Overloads.end(); }
    };

    using FunctionMap = llvm::StringMap<FuncOverloadTable>;
}

#endif //CVOLT_FUNCOVERLOADTABLE_H
