//
// Created by bohdan on 8/4/26.
//

#ifndef CVOLT_FUNCTIONTABLE_H
#define CVOLT_FUNCTIONTABLE_H

#include "Volt/Core/Types/DataType.h"
#include "FuncOverloadTable.h"

namespace Volt
{
    template <typename T>
    struct FunctionTableEntry
    {
        llvm::StringRef Name;
        T& Overload;
    };

    template <typename T>
    struct ConstFunctionTableEntry
    {
        llvm::StringRef Name;
        const T& Overload;
    };

    template <typename FunctionMapTy, typename FuncMapIterTy, typename OverloadIterTy>
    class FunctionTableIteratorBase
    {
    protected:
        FunctionMapTy* Functions;
        FuncMapIterTy FuncMapIter;
        OverloadIterTy OverloadIter;

    public:
        FunctionTableIteratorBase(FunctionMapTy& Functions,
            FuncMapIterTy FuncMapIter, OverloadIterTy OverloadIter)
            : Functions(&Functions), FuncMapIter(FuncMapIter), OverloadIter(OverloadIter) {}

        bool operator==(const FunctionTableIteratorBase& Other) const
        {
            return FuncMapIter == Other.FuncMapIter && OverloadIter == Other.OverloadIter;
        }

        bool operator!=(const FunctionTableIteratorBase& Other) const
        {
            return !(*this == Other);
        }

        FunctionTableIteratorBase& operator++() { Advance(); return *this; }

    protected:
        void Advance();
    };

    template<typename FunctionMapTy, typename FuncMapIterTy, typename OverloadIterTy>
    void FunctionTableIteratorBase<FunctionMapTy, FuncMapIterTy, OverloadIterTy>::Advance()
    {
        if (FuncMapIter == Functions->end())
            return;

        if (++OverloadIter == FuncMapIter->second.end())
        {
            if (++FuncMapIter == Functions->end())
                OverloadIter = OverloadIterTy();
            else
                OverloadIter = FuncMapIter->second.begin();
        }
    }

    template <typename T, size_t N>
    using FuncMap = llvm::StringMap<OverloadTableImpl<T, N>>;

    template <typename T, size_t N>
    class FunctionTableIterator :
        public FunctionTableIteratorBase<FuncMap<T, N>,
            typename FuncMap<T, N>::iterator, typename OverloadTableImpl<T, N>::Iterator>
    {
    public:
        FunctionTableIterator(FuncMap<T, N>& Functions,
            FuncMap<T, N>::iterator FuncMapIter, OverloadTableImpl<T, N>::Iterator OverloadIter)
            : FunctionTableIteratorBase<FuncMap<T, N>, typename FuncMap<T, N>::iterator,
            typename OverloadTableImpl<T, N>::Iterator>(Functions, FuncMapIter, OverloadIter) {}

        FunctionTableEntry<T> operator*() const
        {
            return FunctionTableEntry(
                this->FuncMapIter->first(), *this->OverloadIter);
        }
    };

    template <typename T, size_t N>
    class ConstFunctionTableIterator :
        public FunctionTableIteratorBase<const FuncMap<T, N>,
            typename FuncMap<T, N>::const_iterator, typename OverloadTableImpl<T, N>::ConstIterator>
    {
    public:
        ConstFunctionTableIterator(const FuncMap<T, N>& Functions,
            FuncMap<T, N>::const_iterator FuncMapIter, OverloadTableImpl<T, N>::ConstIterator OverloadIter)
            : FunctionTableIteratorBase<const FuncMap<T, N>, typename FuncMap<T, N>::const_iterator,
            typename OverloadTableImpl<T, N>::ConstIterator>(Functions, FuncMapIter, OverloadIter) {}

        ConstFunctionTableEntry<T> operator*() const
        {
            return ConstFunctionTableEntry(
                this->FuncMapIter->first(), *this->OverloadIter);
        }
    };

    template <typename T, size_t N>
    class FuncTableImpl
    {
    public:
        using OverloadTable = OverloadTableImpl<T, N>;
        using Iterator = FunctionTableIterator<T, N>;
        using ConstIterator = ConstFunctionTableIterator<T, N>;

    private:
        FuncMap<T, N> Functions;

    public:
        template <typename ...ArgsTy>
        void AddFunction(llvm::StringRef Name, ArgsTy&&... Args)
        {
            Functions[Name].AddOverload(std::forward<ArgsTy>(Args)...);
        }

        OverloadResult<T> FindBestFunctionOverload(llvm::StringRef Name, llvm::ArrayRef<QualType> Args) const;

        [[nodiscard]] Iterator begin();
        [[nodiscard]] Iterator end();
        [[nodiscard]] ConstIterator begin() const;
        [[nodiscard]] ConstIterator end() const;
    };

    using FunctionTable = FuncTableImpl<FunctionOverload, 8>;
    using MethodTable = FuncTableImpl<MethodOverload, 8>;
    using BuiltinFuncTable = FuncTableImpl<BuiltinFunctionOverload, 8>;

    template<typename T, size_t N>
    OverloadResult<T> FuncTableImpl<T, N>::FindBestFunctionOverload(
        llvm::StringRef Name, llvm::ArrayRef<QualType> Args) const
    {
        auto Iter = Functions.find(Name);
        if (Iter == Functions.end())
            return OverloadResult<T>{ nullptr, OverloadResultKind::NotAvailable };
        return Iter->second.FindBestOverload(Args);
    }

    template<typename T, size_t N>
    FuncTableImpl<T, N>::Iterator FuncTableImpl<T, N>::begin()
    {
        if (!Functions.empty())
            return Iterator(Functions, Functions.begin(),
               Functions.begin()->second.begin());

        return Iterator(Functions,
            Functions.begin(), typename OverloadTableImpl<T, N>::Iterator());
    }

    template<typename T, size_t N>
    FuncTableImpl<T, N>::Iterator FuncTableImpl<T, N>::end()
    {
        return Iterator(Functions, Functions.end(),
            typename OverloadTableImpl<T, N>::Iterator());
    }

    template<typename T, size_t N>
    FuncTableImpl<T, N>::ConstIterator FuncTableImpl<T, N>::begin() const
    {
        if (!Functions.empty())
            return ConstIterator(Functions, Functions.begin(),
            Functions.begin()->second.begin());

        return ConstIterator(Functions,
        Functions.begin(), typename OverloadTableImpl<T, N>::ConstIterator());
    }

    template<typename T, size_t N>
    FuncTableImpl<T, N>::ConstIterator FuncTableImpl<T, N>::end() const
    {
        return ConstIterator(Functions, Functions.end(),
            typename OverloadTableImpl<T, N>::ConstIterator());
    }
}

#endif //CVOLT_FUNCTIONTABLE_H
