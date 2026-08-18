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

    template <typename T>
    using FuncMap = llvm::StringMap<OverloadTableImpl<T>>;

    template <typename T>
    class FunctionTableIterator :
        public FunctionTableIteratorBase<FuncMap<T>,
            typename FuncMap<T>::iterator, typename OverloadTableImpl<T>::Iterator>
    {
    public:
        FunctionTableIterator(FuncMap<T>& Functions,
            FuncMap<T>::iterator FuncMapIter, OverloadTableImpl<T>::Iterator OverloadIter)
            : FunctionTableIteratorBase<FuncMap<T>, typename FuncMap<T>::iterator,
            typename OverloadTableImpl<T>::Iterator>(Functions, FuncMapIter, OverloadIter) {}

        FunctionTableEntry<T> operator*() const
        {
            return FunctionTableEntry(
                this->FuncMapIter->first(), *this->OverloadIter);
        }
    };

    template <typename T>
    class ConstFunctionTableIterator :
        public FunctionTableIteratorBase<const FuncMap<T>,
            typename FuncMap<T>::const_iterator, typename OverloadTableImpl<T>::ConstIterator>
    {
    public:
        ConstFunctionTableIterator(const FuncMap<T>& Functions,
            FuncMap<T>::const_iterator FuncMapIter, OverloadTableImpl<T>::ConstIterator OverloadIter)
            : FunctionTableIteratorBase<const FuncMap<T>, typename FuncMap<T>::const_iterator,
            typename OverloadTableImpl<T>::ConstIterator>(Functions, FuncMapIter, OverloadIter) {}

        ConstFunctionTableEntry<T> operator*() const
        {
            return ConstFunctionTableEntry(
                this->FuncMapIter->first(), *this->OverloadIter);
        }
    };

    template <typename T>
    class FuncTableImpl
    {
    private:
        FuncMap<T> Functions;

    public:
        void AddFunction(llvm::StringRef Name, ArgsVector<QualType> Params, CalleeBase* Callee)
        {
            Functions[Name].AddOverload(std::move(Params), Callee);
        }

        const T* FindBestFunctionOverload(llvm::StringRef Name, llvm::ArrayRef<QualType> Args) const;

        [[nodiscard]] FunctionTableIterator<T> begin();
        [[nodiscard]] FunctionTableIterator<T> end();
        [[nodiscard]] ConstFunctionTableIterator<T> begin() const;
        [[nodiscard]] ConstFunctionTableIterator<T> end() const;
    };

    using FunctionTable = FuncTableImpl<FunctionOverload>;
    using MethodTable = FuncTableImpl<MethodOverload>;

    template<typename T>
    const T* FuncTableImpl<T>::FindBestFunctionOverload(llvm::StringRef Name, llvm::ArrayRef<QualType> Args) const
    {
        auto Iter = Functions.find(Name);
        if (Iter == Functions.end()) return nullptr;
        return Iter->second.FindBestOverload(Args);
    }

    template<typename T>
    FunctionTableIterator<T> FuncTableImpl<T>::begin()
    {
        if (!Functions.empty())
            return FunctionTableIterator(Functions, Functions.begin(),
               Functions.begin()->second.begin());

        return FunctionTableIterator(Functions,
            Functions.begin(), typename OverloadTableImpl<T>::Iterator());
    }

    template<typename T>
    FunctionTableIterator<T> FuncTableImpl<T>::end()
    {
        return FunctionTableIterator(Functions, Functions.end(),
            typename OverloadTableImpl<T>::Iterator());
    }

    template<typename T>
    ConstFunctionTableIterator<T> FuncTableImpl<T>::begin() const
    {
        if (!Functions.empty())
            return ConstFunctionTableIterator(Functions, Functions.begin(),
            Functions.begin()->second.begin());

        return ConstFunctionTableIterator(Functions,
        Functions.begin(), typename OverloadTableImpl<T>::ConstIterator());
    }

    template<typename T>
    ConstFunctionTableIterator<T> FuncTableImpl<T>::end() const
    {
        return ConstFunctionTableIterator(Functions, Functions.end(),
            typename OverloadTableImpl<T>::ConstIterator());
    }
}

#endif //CVOLT_FUNCTIONTABLE_H
