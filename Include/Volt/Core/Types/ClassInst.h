//
// Created by bohdan on 20.07.26.
//

#ifndef CVOLT_CLASSINST_H
#define CVOLT_CLASSINST_H

#include "ClassType.h"
#include "TypeConv.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Utils/IRNameBuilder.h"

namespace Volt
{
    template <typename RetTy, typename ...ArgsTy>
    using MethodTy = RetTy(*)(void*, ArgsTy...);

    template <typename RetTy, typename ...ArgsTy>
    class ClassMethod
    {
    private:
        using MethodTy = MethodTy<RetTy, ArgsTy...>;

    private:
        void* Data = nullptr;
        MethodTy Method = nullptr;

        ClassMethod(void* Data, MethodTy Method)
            : Data(Data), Method(Method) {}

    public:
        ClassMethod() = default;

        RetTy operator()(ArgsTy... Args)
        {
            VoltAssert(Data != nullptr && Method != nullptr);
            return Method(Data, Args...);
        }

        friend class ClassInstBase;
    };

    class ClassInstBase
    {
    protected:
        ClassType* Type = nullptr;
        char* Data = nullptr;
        llvm::StringMap<void*> Methods;

        CompilationContext& CContext;

        ClassInstBase(ClassType* Type, CompilationContext& CContext)
            : Type(Type), CContext(CContext) { }

    public:
        [[nodiscard]] ClassType* GetType() const { return Type; }
        [[nodiscard]] void *GetData() const { return Data; }

        [[nodiscard]] void *GetField(llvm::StringRef FieldName, size_t Size) const;
        template <typename T>
        T& GetField(llvm::StringRef FieldName);

        [[nodiscard]] class ClassInstView GetField(llvm::StringRef FieldName) const;

        template <typename RetTy, typename ...ArgsTy>
        ClassMethod<RetTy, ArgsTy...> GetMethodAddr(llvm::StringRef Name);

        template <typename RetTy, typename ...ArgsTy>
        RetTy CallMethod(llvm::StringRef Name, ArgsTy... Args);

        friend class JITEngine;
    };

    class ClassInst : public ClassInstBase
    {
    public:
        ClassInst(ClassType* Type, CompilationContext& CContext)
            : ClassInstBase(Type, CContext)
        {
            Data = static_cast<char*>(operator new(Type->GetSize()));
        }

        ClassInst(const ClassInst& Other);
        ClassInst& operator=(const ClassInst& Other);

        ClassInst(ClassInst&& Other) noexcept;
        ClassInst& operator=(ClassInst&& Other) noexcept;

        ~ClassInst() { operator delete(Data); }
    };

    class ClassInstView : public ClassInstBase
    {
    public:
        ClassInstView(ClassType* Type, char* InData, CompilationContext& CContext)
            : ClassInstBase(Type, CContext) { Data = InData; }
    };

    template<typename T>
    T& ClassInstBase::GetField(llvm::StringRef FieldName)
    {
        size_t FieldIndex = Type->GetFieldIndex(FieldName);
        VoltAssert(FieldIndex != Type->Fields.Length());
        const Field& F = Type->Fields[FieldIndex];
        QualType Ty = TypeConv::GetDataType<T>(CContext);
        if (Ty != F.Type)
            VoltUnreachableFmt("Cannot get field with type '{}' as '{}'", F.Type.ToString(), Ty.ToString());
        return *reinterpret_cast<T*>(Data + F.Offset);
    }

    template<typename RetTy, typename ... ArgsTy>
    ClassMethod<RetTy, ArgsTy...> ClassInstBase::GetMethodAddr(llvm::StringRef Name)
    {
        IRNameBuilder NameBuilder(IRNameKind::Method);
        NameBuilder.AddName(Type->Name);
        NameBuilder.AddName(Name);

        NameBuilder.AddParam(CContext.GetPointerType(Type));
        if constexpr (sizeof...(ArgsTy) > 0)
            NameBuilder.AddParams<ArgsTy...>(CContext);

        if (auto Iter = Methods.find(NameBuilder.GetIRName()); Iter != Methods.end())
            return ClassMethod<RetTy, ArgsTy...>(
                Data, reinterpret_cast<MethodTy<RetTy, ArgsTy...>>(Iter->second));

        VoltUnreachableFmt("Cannot find method '{}'", NameBuilder.GetIRName());
    }

    template<typename RetTy, typename ... ArgsTy>
    RetTy ClassInstBase::CallMethod(llvm::StringRef Name, ArgsTy... Args)
    {
        return GetMethodAddr<RetTy, ArgsTy...>(Name)(Args...);
    }
}

#endif //CVOLT_CLASSINST_H
