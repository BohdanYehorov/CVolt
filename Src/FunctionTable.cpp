//
// Created by bohdan on 8/4/26.
//

#include "Volt/Core/Functions/FunctionTable.h"

namespace Volt
{
    const FunctionOverload* FunctionTable::FindBestFunctionOverload(
        llvm::StringRef Name, llvm::ArrayRef<QualType> Args) const
    {
        auto Iter = Functions.find(Name);
        if (Iter == Functions.end()) return nullptr;
        return Iter->second.FindBestOverload(Args);
    }

    FunctionTableIterator FunctionTable::begin()
    {
        if (!Functions.empty())
            return FunctionTableIterator(Functions, Functions.begin(),
               Functions.begin()->second.begin());

        return FunctionTableIterator(Functions,
            Functions.begin(), FuncOverloadTable::Iterator());
    }

    FunctionTableIterator FunctionTable::end()
    {
        return FunctionTableIterator(Functions, Functions.end(),
            FuncOverloadTable::Iterator());
    }

    ConstFunctionTableIterator FunctionTable::begin() const
    {
        if (!Functions.empty())
            return ConstFunctionTableIterator(Functions, Functions.begin(),
            Functions.begin()->second.begin());

        return ConstFunctionTableIterator(Functions,
        Functions.begin(), FuncOverloadTable::ConstIterator());
    }

    ConstFunctionTableIterator FunctionTable::end() const
    {
        return ConstFunctionTableIterator(Functions, Functions.end(),
                                          FuncOverloadTable::ConstIterator());
    }
}
