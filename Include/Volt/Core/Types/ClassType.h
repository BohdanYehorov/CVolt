//
// Created by bohdan on 21.06.26.
//

#ifndef CVOLT_CLASSTYPE_H
#define CVOLT_CLASSTYPE_H

#include "DataType.h"
#include "Volt/Core/TypeDefs/UMap.h"

namespace Volt
{
    struct Field
    {
        std::string Name;
        QualType Type;

        Field(std::string Name, QualType Type)
            : Name(std::move(Name)), Type(Type) {}
    };

    class ClassType : public DataType
    {
        GENERATED_BODY(ClassType, DataType)
    public:
        std::string Name;
        Array<Field> Fields;
        FunctionTable Methods;

    public:
        ClassType(std::string Name, Array<Field> Fields)
            : DataType(TypeCategory::CLASS), Name(std::move(Name)),
            Fields(std::move(Fields)) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override;
        int GetRank() const override { return -1; }
        std::string ToString() const override { return Name; }
        bool CastTo(DataType *To, bool Explicit) const override { return this == To; }

        size_t GetFieldIndex(const std::string& Name);

        void AddMethod(FunctionSignature Signature, FunctionCallee* Callee)
        {
            Methods[std::move(Signature)] = Callee;
        }
    };
}

#endif //CVOLT_CLASSTYPE_H
