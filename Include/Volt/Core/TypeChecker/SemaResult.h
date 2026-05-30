//
// Created by bohdan on 17.05.26.
//

#ifndef CVOLT_SEMARESULT_H
#define CVOLT_SEMARESULT_H

#include "Volt/Core/Object/Object.h"
#include "Volt/Core/Types/DataType.h"

namespace Volt
{
    class SemaResult : public Object
    {
        GENERATED_BODY(SemaResult, Object)

    protected:
        QualType Type = nullptr;

    public:
        SemaResult() = default;
        SemaResult(QualType Type) : Type(Type) {}

        [[nodiscard]] QualType GetType() const { return Type; }
    };
}

#endif //CVOLT_SEMARESULT_H
