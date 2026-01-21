//
// Created by bohdan on 14.12.25.
//

#ifndef CVOLT_OBJECT_H
#define CVOLT_OBJECT_H
#include <string>

namespace Volt
{
    class Object
    {
    protected:
        static size_t GenerateType() { static size_t Id = 0; return ++Id; }

    public:
        virtual ~Object() = default;

        static size_t Object_StaticType() { static size_t Id = GenerateType(); return Id; }
        [[nodiscard]] virtual size_t Object_GetType() const { return  Object_StaticType(); };
        [[nodiscard]] virtual bool Object_IsA(size_t Type) const { return Type == Object_StaticType(); };
        [[nodiscard]] virtual std::string GetName() const { return "Object"; }
    };

    #define GENERATED_BODY(Object, Base) \
    public:\
        using Super = Base; \
        static size_t Object_StaticType() { static size_t Id = GenerateType(); return Id; } \
        size_t Object_GetType() const override { return Object_StaticType(); } \
        bool Object_IsA(size_t Type_) const override \
        { return Type_ == Object_StaticType() || Base::Object_IsA(Type_); } \
        std::string GetName() const override { return #Object; }

    template<typename To, typename From>
    To* Cast(From* Obj)
    {
        if (Obj && Obj->Object_IsA(To::Object_StaticType()))
            return reinterpret_cast<To*>(Obj);
        return nullptr;
    }
}

#endif //CVOLT_OBJECT_H