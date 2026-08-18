//
// Created by bohdan on 8/4/26.
//

#ifndef CVOLT_FUNCOVERLOADTABLE_H
#define CVOLT_FUNCOVERLOADTABLE_H

#include "Volt/Core/Functions/FunctionOverload.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"

namespace Volt
{
    using FuncOverloadVector = SmallVec8<FunctionOverload>;

    template <typename T>
    class OverloadTableImpl
    {
    public:
        using Iterator = FuncOverloadVector::iterator;
        using ConstIterator = FuncOverloadVector::const_iterator;
        using OverloadVector = SmallVec8<T>;

    private:
        OverloadVector Overloads;

    public:
        void AddOverload(ArgsVector<QualType> Args, CalleeBase* Callee)
        {
            Overloads.emplace_back(std::move(Args), Callee);
        }

        [[nodiscard]] const T* FindBestOverload(llvm::ArrayRef<QualType> Args) const;

        [[nodiscard]] Iterator begin() { return Overloads.begin(); }
        [[nodiscard]] Iterator end() { return Overloads.end(); }
        [[nodiscard]] ConstIterator begin() const { return Overloads.begin(); }
        [[nodiscard]] ConstIterator end() const { return Overloads.end(); }
    };

    template<typename T>
    const T * OverloadTableImpl<T>::FindBestOverload(llvm::ArrayRef<QualType> Args) const
    {
        size_t ArgsCount = Args.size();
        size_t MinCasts = ArgsCount;
        int BestRank = std::numeric_limits<int>::max();
        const T* BestOverload = nullptr;

        for (const T& Overload : Overloads)
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

    using FuncOverloadTable = OverloadTableImpl<FunctionOverload>;
    using MethodOverloadTable = OverloadTableImpl<MethodOverload>;

    using FunctionMap = llvm::StringMap<FuncOverloadTable>;
}

#endif //CVOLT_FUNCOVERLOADTABLE_H
