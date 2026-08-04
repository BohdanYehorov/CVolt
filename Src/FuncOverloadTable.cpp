//
// Created by bohdan on 8/4/26.
//

#include "Volt/Core/Functions/FuncOverloadTable.h"

namespace Volt
{
    const FunctionOverload *FuncOverloadTable::FindBestOverload(llvm::ArrayRef<QualType> Args) const
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
