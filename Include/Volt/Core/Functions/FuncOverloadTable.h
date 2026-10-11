//
// Created by bohdan on 8/4/26.
//

#ifndef CVOLT_FUNCOVERLOADTABLE_H
#define CVOLT_FUNCOVERLOADTABLE_H

#include "Volt/Core/Functions/FunctionOverload.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Core/Memory/Arena.h"
#include "Volt/ADT/PointerIntPair.h"

namespace Volt
{
    using FuncOverloadVector = SmallVec8<FunctionOverload>;

    enum class OverloadResultKind : UInt8
    {
        Valid,
        Ambiguous,
        NotAvailable
    };

    template <typename T>
    struct OverloadResult
    {
    private:
        PointerIntPair<const T, alignof(UInt32), OverloadResultKind> Value;

    public:
        OverloadResult(const T* Overload, OverloadResultKind Kind)
            : Value(Overload, Kind) {}

        [[nodiscard]] const T* GetOverload() const { return Value.GetPointer(); }
        [[nodiscard]] OverloadResultKind GetKind() const { return Value.GetInt(); }
    };

    template <typename T>
    class OverloadTableImpl
    {
        struct OverloadData
        {
            const T* Overload;
            llvm::ArrayRef<CastKind> CastKinds;

            OverloadData(const T* Overload, llvm::ArrayRef<CastKind> CastKinds)
                : Overload(Overload), CastKinds(CastKinds) {}
        };

    public:
        using OverloadVector = SmallVec8<T>;
        using Iterator = OverloadVector::iterator;
        using ConstIterator = OverloadVector::const_iterator;

    private:
        OverloadVector Overloads;

    public:
        template <typename ...ArgsTy>
        void AddOverload(ArgsTy&&... Args)
        {
            Overloads.emplace_back(std::forward<ArgsTy>(Args)...);
        }

        [[nodiscard]] OverloadResult<T> FindBestOverload(llvm::ArrayRef<QualType> Args) const;

        [[nodiscard]] Iterator begin() { return Overloads.begin(); }
        [[nodiscard]] Iterator end() { return Overloads.end(); }
        [[nodiscard]] ConstIterator begin() const { return Overloads.begin(); }
        [[nodiscard]] ConstIterator end() const { return Overloads.end(); }
    };

    [[nodiscard]] bool Dominates(llvm::ArrayRef<CastKind> A, llvm::ArrayRef<CastKind> B);

    template<typename T>
    OverloadResult<T> OverloadTableImpl<T>::FindBestOverload(llvm::ArrayRef<QualType> Args) const
    {
        SmallVec4<OverloadData> BestOverloads;
        Arena CastKindsArena;
        for (const T& Overload : Overloads)
        {
            Array<CastKind> CastKinds;
            if (!Overload.GetCastKindsAndCheckIsValidCasts(Args, CastKinds))
                continue;

            bool ExistingDominates = false;
            BestOverloads.erase(std::remove_if(BestOverloads.begin(), BestOverloads.end(),
                [&](const OverloadData& Existing) {
                    if (Dominates(CastKinds, Existing.CastKinds)) return true;
                    if (Dominates(Existing.CastKinds, CastKinds)) ExistingDominates = true;
                    return false;
                }), BestOverloads.end());
            if (!ExistingDominates)
                BestOverloads.emplace_back(&Overload,
                    CastKindsArena.AllocStaticArray(std::move(CastKinds)));
        }

        if (BestOverloads.empty())
            return OverloadResult<T>{ nullptr, OverloadResultKind::NotAvailable };
        if (BestOverloads.size() > 1)
            return OverloadResult<T>{ BestOverloads[0].Overload, OverloadResultKind::Ambiguous };
        return OverloadResult<T>{ BestOverloads[0].Overload, OverloadResultKind::Valid };
    }

    using FuncOverloadTable = OverloadTableImpl<FunctionOverload>;
    using MethodOverloadTable = OverloadTableImpl<MethodOverload>;

    using FunctionMap = llvm::StringMap<FuncOverloadTable>;
}

#endif //CVOLT_FUNCOVERLOADTABLE_H
