//
// Created by bohdan on 8/25/26.
//

#include "Volt/Core/Functions/FuncOverloadTable.h"

namespace Volt
{
    bool Dominates(llvm::ArrayRef<CastKind> A, llvm::ArrayRef<CastKind> B)
    {
        VoltAssert(A.size() == B.size());
        bool Dominates = false;
        for (size_t i = 0; i < A.size(); i++)
        {
            size_t FirstRank = static_cast<size_t>(A[i]);
            size_t SecondRank = static_cast<size_t>(B[i]);

            if (FirstRank > SecondRank) return false;
            if (FirstRank < SecondRank) Dominates = true;
        }

        return Dominates;
    }
}
