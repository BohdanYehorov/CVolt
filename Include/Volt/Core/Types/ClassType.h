//
// Created by bohdan on 21.06.26.
//

#ifndef CVOLT_CLASSTYPE_H
#define CVOLT_CLASSTYPE_H

#include "DataType.h"
#include "Volt/Core/TypeDefs/UMap.h"
#include "Volt/ADT/Array.h"

namespace Volt
{
    struct Field
    {
        std::string Name;
        QualType Type;
        size_t Offset = 0;

        Field(std::string Name, QualType Type)
            : Name(std::move(Name)), Type(Type) {}
    };

    class ClassType : public DataType
    {
        GENERATED_BODY(ClassType, DataType)
    public:
        using ConstructorTable = SmallVec8<std::pair<SmallVec4<QualType>, FunctionCallee*>>;

    public:
        std::string Name;
        Array<Field> Fields;
        FunctionTable Methods;
        ConstructorTable Constructors;
        mutable size_t Size = 0;
        mutable size_t Alignment = 0;

    public:
        ClassType(std::string Name, Array<Field> Fields)
            : DataType(TypeCategory::CLASS), Name(std::move(Name)),
            Fields(std::move(Fields)) { ComputeLayout(); }

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override;
        int GetRank() const override { return -1; }
        std::string ToString() const override { return Name; }
        size_t GetSize() const override;
        size_t GetAlignment() const override;
        std::string GetIRName() const override { return std::to_string(Name.size()) + Name; }

        void ComputeLayout() const;

        bool CastTo(DataType *To, bool Explicit) const override { return this == To; }

        size_t GetFieldIndex(const std::string& Name);

        void AddMethod(llvm::StringRef Name, ArgsVector<QualType> Params, FunctionCallee* Callee)
        {
            Methods[Name].emplace_back(std::move(Params), Callee);
        }

    private:
        static size_t AlignUp(size_t Offset, size_t Align)
        {
            return (Offset + Align - 1) & ~(Align - 1);
        }
    };
}

#endif //CVOLT_CLASSTYPE_H
