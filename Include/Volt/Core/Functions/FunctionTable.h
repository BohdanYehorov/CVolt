//
// Created by bohdan on 8/4/26.
//

#ifndef CVOLT_FUNCTIONTABLE_H
#define CVOLT_FUNCTIONTABLE_H

#include "Volt/Core/Types/DataType.h"
#include "FuncOverloadTable.h"

namespace Volt
{
    struct FunctionTableEntry
    {
        llvm::StringRef Name;
        FunctionOverload& Overload;
    };

    struct ConstFunctionTableEntry
    {
        llvm::StringRef Name;
        const FunctionOverload& Overload;
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

    class FunctionTableIterator :
        public FunctionTableIteratorBase<FunctionMap, FunctionMap::iterator, FuncOverloadTable::Iterator>
    {
    public:
        FunctionTableIterator(FunctionMap& Functions,
            FunctionMap::iterator FuncMapIter, FuncOverloadTable::Iterator OverloadIter)
            : FunctionTableIteratorBase(Functions, FuncMapIter, OverloadIter) {}

        FunctionTableEntry operator*() const
        {
            return FunctionTableEntry(FuncMapIter->first(), *OverloadIter);
        }
    };

    class ConstFunctionTableIterator :
    public FunctionTableIteratorBase<const FunctionMap, FunctionMap::const_iterator, FuncOverloadTable::ConstIterator>
    {
    public:
        ConstFunctionTableIterator(const FunctionMap& Functions,
            FunctionMap::const_iterator FuncMapIter, FuncOverloadTable::ConstIterator OverloadIter)
            : FunctionTableIteratorBase(Functions, FuncMapIter, OverloadIter) {}

        ConstFunctionTableEntry operator*() const
        {
            return ConstFunctionTableEntry(FuncMapIter->first(), *OverloadIter);
        }
    };

    class FunctionTable
    {
    private:
        FunctionMap Functions;

    public:
        void AddFunction(llvm::StringRef Name, ArgsVector<QualType> Params, CalleeBase* Callee)
        {
            Functions[Name].AddOverload(std::move(Params), Callee);
        }

        const FunctionOverload* FindBestFunctionOverload(llvm::StringRef Name, llvm::ArrayRef<QualType> Args) const;

        [[nodiscard]] FunctionTableIterator begin();
        [[nodiscard]] FunctionTableIterator end();
        [[nodiscard]] ConstFunctionTableIterator begin() const;
        [[nodiscard]] ConstFunctionTableIterator end() const;
    };
}

#endif //CVOLT_FUNCTIONTABLE_H
