//
// Created by bohdan on 23.07.26.
//

// +---------------------------------------------------------+
// |   type   |                mangled name                  |
// |----------|----------------------------------------------|
// | void     | v                                            |
// |----------|----------------------------------------------|
// | bool     | b                                            |
// |----------|----------------------------------------------|
// | char     | c                                            |
// |----------|----------------------------------------------|
// | i8       | k                                            |
// |----------|----------------------------------------------|
// | i16      | s                                            |
// |----------|----------------------------------------------|
// | i32      | i                                            |
// |----------|----------------------------------------------|
// | i64      | l                                            |
// |----------|----------------------------------------------|
// | u8       | r                                            |
// |----------|----------------------------------------------|
// | u16      | t                                            |
// |----------|----------------------------------------------|
// | u32      | u                                            |
// |----------|----------------------------------------------|
// | u64      | m                                            |
// |----------|----------------------------------------------|
// | f16      | h                                            |
// |----------|----------------------------------------------|
// | f32      | f                                            |
// |----------|----------------------------------------------|
// | f64      | d                                            |
// |----------|----------------------------------------------|
// | f128     | q                                            |
// |----------|----------------------------------------------|
// | ptr      | P + <base>                                   |
// |----------|----------------------------------------------|
// | nulltype | n                                            |
// |----------|----------------------------------------------|
// | ref      | R + <base>                                   |
// |----------|----------------------------------------------|
// | arr      | A + <len> + <base>                           |
// |----------|----------------------------------------------|
// | class    | <class name>                                 |
// |----------|----------------------------------------------|
// | function | F + <ret type> + <params> + E                |
// |----------|----------------------------------------------|
// | method   | M + <class name> + <ret type> + <params> + E |
// +---------------------------------------------------------+

#ifndef CVOLT_NAMEMANGLER_H
#define CVOLT_NAMEMANGLER_H

#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Types/ClassType.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"
#include "Volt/Core/Types/TypeConv.h"

namespace Volt
{
    enum class MangleKind
    {
        Function,
        Method
    };

    // Mangling functions: F + FunctionName + Args
    // Mangling methods:   M + OwnerClassName + FunctionName + Args
    class NameMangler
    {
    public:
        static std::string Mangle(llvm::StringRef Name, FunctionType* FuncType, ClassType* Owner = nullptr);
        static std::string Mangle(llvm::StringRef Name, CalleeBase* Callee);

        template <typename ...Args>
        static std::string Mangle(llvm::StringRef Name, CompilationContext& CContext, ClassType* Owner = nullptr);

    private:
        std::string MangledName;

    public:
        NameMangler(MangleKind Kind)
        {
            MangledName = Kind == MangleKind::Function ? "F" : "M";
        }

        void AddName(const llvm::StringRef Name)
        {
            MangledName += std::to_string(Name.size());
            MangledName.append(Name.data(), Name.size());
        }
        void AddParam(QualType Param) { MangledName += Param.GetMangledName(); }
        void AddParams(llvm::ArrayRef<QualType> Params) { for (auto P : Params) AddParam(P); }

        template <typename T>
        void AddParam(CompilationContext& CContext)
        {
            AddParam(TypeConv::GetDataType<T>(CContext));
        }

        template <typename T, typename ...ArgsTy>
        void AddParams(CompilationContext& CContext);

        [[nodiscard]] const std::string& GetMangledName() const { return MangledName; }
    };

    template<typename ... ArgsTy>
    std::string NameMangler::Mangle(llvm::StringRef Name, CompilationContext& CContext, ClassType *Owner)
    {
        bool IsFunction = Owner == nullptr;
        NameMangler Mangler(IsFunction ? MangleKind::Function : MangleKind::Method);
        if (Owner) Mangler.AddName(Owner->GetName());
        Mangler.AddName(Name);
        if constexpr (sizeof...(ArgsTy) > 0)
            Mangler.AddParams<ArgsTy...>(CContext);
        return Mangler.MangledName;
    }

    template<typename T, typename ... ArgsTy>
    void NameMangler::AddParams(CompilationContext &CContext)
    {
        AddParam<T>(CContext);
        if constexpr (sizeof...(ArgsTy) > 0)
            AddParams<ArgsTy...>(CContext);
    }
}

#endif //CVOLT_NAMEMANGLER_H
