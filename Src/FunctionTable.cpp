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
        return FindBestOverload(Args, Iter->second);
    }

    FunctionTableIterator FunctionTable::begin()
    {
        if (!Functions.empty())
            return FunctionTableIterator(Functions, Functions.begin(),
               Functions.begin()->second.begin());

        return FunctionTableIterator(Functions,
            Functions.begin(), FuncOverloadVector::iterator());
    }

    FunctionTableIterator FunctionTable::end()
    {
        return FunctionTableIterator(Functions, Functions.end(),
            FuncOverloadVector::iterator());
    }

    ConstFunctionTableIterator FunctionTable::begin() const
    {
        if (!Functions.empty())
            return ConstFunctionTableIterator(Functions, Functions.begin(),
            Functions.begin()->second.begin());

        return ConstFunctionTableIterator(Functions,
        Functions.begin(), FuncOverloadVector::const_iterator());
    }

    ConstFunctionTableIterator FunctionTable::end() const
    {
        return ConstFunctionTableIterator(Functions, Functions.end(),
                                          FuncOverloadVector::iterator());
    }

    const FunctionOverload* FunctionTable::FindBestOverload(
        llvm::ArrayRef<QualType> Args, const FuncOverloadVector &Overloads)
    {
        size_t ArgsCount = Args.size();
        size_t MinCasts = ArgsCount;
        int BestRank = std::numeric_limits<int>::max();
        const FunctionOverload* BestOverload = nullptr;

        for (const FunctionOverload& Overload : Overloads)
        {
            if (Overload.Args.size() != ArgsCount) continue;

            int RankDiff = 0;
            size_t Casts = 0;
            bool Valid = true;
            for (size_t i = 0; i < ArgsCount; i++)
            {
                QualType CandidateArgType = Overload.Args[i];
                QualType ArgType = Args[i];

                if (auto RefType = CandidateArgType.CastAs<ReferenceType>())
                {
                    if (RefType->CanBind(ArgType))
                        continue;

                    Valid = false;
                    break;
                }

                if (!ArgType.ImplicitCast(CandidateArgType))
                {
                    Valid = false;
                    break;
                }

                if (ArgType != CandidateArgType)
                    Casts++;

                RankDiff += std::abs(
                    CandidateArgType->GetRank() - ArgType->GetRank());
            }

            if (!Valid) continue;

            if (!BestOverload || Casts < MinCasts || (Casts == MinCasts && RankDiff < BestRank))
            {
                MinCasts = Casts;
                BestRank = RankDiff;
                BestOverload = &Overload;
            }
        }

        return BestOverload;
    }
}
