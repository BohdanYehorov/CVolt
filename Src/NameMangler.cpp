//
// Created by bohdan on 9/19/26.
//
#include "Volt/Utils/NameMangler.h"

namespace Volt
{
    std::string NameMangler::Mangle(llvm::StringRef Name, FunctionType *FuncType, ClassType *Owner)
    {
        bool IsFunction = Owner == nullptr;
        NameMangler Mangler(IsFunction ? MangleKind::Function : MangleKind::Method);
        if (!IsFunction) Mangler.AddName(Owner->GetName());
        Mangler.AddName(Name);
        Mangler.AddParams(FuncType->GetParams());
        return Mangler.MangledName;
    }

    std::string NameMangler::Mangle(llvm::StringRef Name, CalleeBase *Callee)
    {
        ClassType* Owner = nullptr;
        if (auto MC = Cast<MethodCallee>(Callee))
            Owner = MC->Owner;

        return Mangle(Name, Callee->FuncType, Owner);
    }
}
