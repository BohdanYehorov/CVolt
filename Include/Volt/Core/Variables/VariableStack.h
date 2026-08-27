//
// Created by bohdan on 8/27/26.
//

#ifndef CVOLT_VARIABLESTACK_H
#define CVOLT_VARIABLESTACK_H

#include "Volt/ADT/Array.h"
#include <llvm/ADT/StringMap.h>
#include "Volt/Core/TypeDefs/TypeDefs.h"
#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include "Volt/ADT/PointerIntPair.h"

namespace Volt
{
    template <typename T>
    class VariableStack
    {
        struct ScopeEntry
        {
            llvm::StringRef Name;
            T* Prev = nullptr;

            ScopeEntry(const llvm::StringRef Name, T* Prev)
                : Name(Name), Prev(Prev) {}
        };

    public:
        enum VariableDeclKind : UInt8
        {
            Valid = 0,
            AlreadyExists
        };

    private:
        llvm::StringMap<T*> Variables;
        std::vector<SmallVec8<ScopeEntry>> ScopeStack;

    public:
        void EnterScope() { ScopeStack.emplace_back(); }
        void ExitScope();

        VariableDeclKind DeclareVariable(llvm::StringRef Name, T* Var);
        T* GetVariable(llvm::StringRef Name);
    };

    template<typename T>
    void VariableStack<T>::ExitScope()
    {
        for (const auto& Entry : ScopeStack.back())
        {
            if (Entry.Prev)
                Variables[Entry.Name] = Entry.Prev;
            else
                Variables.erase(Entry.Name);
        }

        ScopeStack.pop_back();
    }

    template<typename T>
    VariableStack<T>::VariableDeclKind VariableStack<T>::DeclareVariable(llvm::StringRef Name, T *Var)
    {
        VoltAssert(!ScopeStack.empty() && "Cannot declare variable: scope is missing");

        if (auto Iter = std::find_if(
            ScopeStack.back().begin(), ScopeStack.back().end(),
            [&Name](const ScopeEntry& Entry) -> bool
            {
                return Entry.Name == Name;
            });
            Iter != ScopeStack.back().end())
            return VariableDeclKind::AlreadyExists;

        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            ScopeStack.back().emplace_back(Name, Iter->second);
        else
            ScopeStack.back().emplace_back(Name, nullptr);

        Variables[Name] = Var;
        return VariableDeclKind::Valid;
    }

    template<typename T>
    T *VariableStack<T>::GetVariable(llvm::StringRef Name)
    {
        if (auto Iter = Variables.find(Name); Iter != Variables.end())
            return Iter->getValue();
        return nullptr;
    }
}

#endif //CVOLT_VARIABLESTACK_H
