//
// Created by bohdan on 23.07.26.
//

#ifndef CVOLT_IRNAMEBUILDER_H
#define CVOLT_IRNAMEBUILDER_H

#include "Volt/Core/Types/DataType.h"
#include "Volt/Core/Types/ClassType.h"

namespace Volt
{
    enum class IRNameKind
    {
        Function,
        Method
    };

    class IRNameBuilder
    {
    private:
        std::string IRName;
        IRNameKind Kind;

    public:
        IRNameBuilder(IRNameKind Kind) : Kind(Kind)
        {
            IRName = Kind == IRNameKind::Function ? "F" : "M";
        }

        IRNameBuilder(const FunctionSignature& Signature)
            : IRName("F"), Kind(IRNameKind::Function)
        {
            AddSignature(Signature);
        }

        IRNameBuilder(ClassType* Type, const FunctionSignature& Signature)
            : Kind(IRNameKind::Method)
        {
            IRName = "M" + std::to_string(Type->Name.size());
            IRName.append(Type->Name.data(), Type->Name.size());
            AddSignature(Signature);
        }

        void AddName(const llvm::StringRef Name)
        {
            IRName += std::to_string(Name.size());
            IRName.append(Name.data(), Name.size());
        }
        void AddParam(QualType Param) { IRName += Param.GetIRName(); }

        template <typename T>
        void AddParam(CompilationContext& CContext)
        {
            AddParam(TypeConv::GetDataType<T>(CContext));
        }

        template <typename T, typename ...ArgsTy>
        void AddParams(CompilationContext& CContext);

        [[nodiscard]] const std::string& GetIRName() const { return IRName; }

    private:
        void AddSignature(const FunctionSignature& Signature)
        {
            AddName(Signature.Name);
            for (const auto& Param : Signature.Params)
                AddParam(Param);
        }
    };

    template<typename T, typename ... ArgsTy>
    void IRNameBuilder::AddParams(CompilationContext &CContext)
    {
        AddParam<T>(CContext);
        if constexpr (sizeof...(ArgsTy) > 0)
            AddParams<ArgsTy...>(CContext);
    }
}

#endif //CVOLT_IRNAMEBUILDER_H
