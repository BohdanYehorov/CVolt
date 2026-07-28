//
// Created by bohdan on 7/26/26.
//

#ifndef CVOLT_VOLTCLASS_H
#define CVOLT_VOLTCLASS_H

#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "TypeConv.h"
#include <utility>

namespace Volt
{
    template <typename FieldTy, typename ClassTy>
    class ClassField
    {
    private:
        FieldTy Value;

    public:
        template <typename ...ArgsTy>
        ClassField(const std::string& Name, ArgsTy&&... Args)
            : Value(std::forward<ArgsTy...>(Args)...)
        {
            if (!ClassTy::ClassRegistered)
                ClassTy::Fields.Emplace(
                   Name, TypeConv::GetBaseType<FieldTy>(*ClassTy::CContext));
        }

        template <typename T>
        explicit operator T() { return static_cast<T>(Value); }
        operator FieldTy() { return Value; }
        operator FieldTy() const { return Value; }
    };

    class VoltClassStaticFields
    {
    protected:
        static CompilationContext* CContext;

    public:
        static void SetContext(CompilationContext* InCContext) { CContext = InCContext; }
    };

    template <typename Derived>
    class VoltClass : public VoltClassStaticFields
    {
    public:
        template <typename, typename>
        friend class ClassField;

        template <typename FieldTy>
        using ClassField = ClassField<FieldTy, Derived>;

    protected:
        static Array<Field> Fields;
        static UMap<FunctionSignature, void*> Methods;
        static bool ClassRegistered;

    public:
        static void RegisterClass()
        {
            VoltAssert(CContext != nullptr);
            Derived();
            Derived::CreateClassType();
        }

        static ClassType* GetClassType()
        {
            VoltUnreachable("Cannot get ClassType in VoltClass");
        }

    public:
        VoltClass() { ClassRegistered = true; }
    };

    template <typename Derived>
    Array<Field> VoltClass<Derived>::Fields;

    template <typename Derived>
    UMap<FunctionSignature, void*> VoltClass<Derived>::Methods;

    template <typename Derived>
    bool VoltClass<Derived>::ClassRegistered = false;

#define GENERATED_VOLT_CLASS_BODY(ClassName)                           \
        static_assert(std::is_class_v<ClassName>);                     \
    public:                                                            \
        static Volt::ClassType* GetClassType()                               \
        { return CContext->GetOrCreateClassType(#ClassName, Fields); } \
        static void CreateClassType()                                  \
        { CContext->CreateClassType(#ClassName, Fields); }

#define FIELD(Type, Name, ...) ClassField<Type> Name{ #Name, ##__VA_ARGS__ }
}

#endif //CVOLT_VOLTCLASS_H
