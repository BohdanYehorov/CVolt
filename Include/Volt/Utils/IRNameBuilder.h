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
            IRName = "M" + std::to_string(Type->Name.size()) + Type->Name;
            AddSignature(Signature);
        }

        void AddName(const std::string& Name) { IRName += std::to_string(Name.size()) + Name; }
        void AddParam(QualType Param) { IRName += Param.GetIRName(); }

        [[nodiscard]] const std::string& GetIRName() const { return IRName; }

    private:
        void AddSignature(const FunctionSignature& Signature)
        {
            AddName(Signature.Name);
            for (const auto& Param : Signature.Params)
                AddParam(Param);
        }
    };
}

#endif //CVOLT_IRNAMEBUILDER_H
