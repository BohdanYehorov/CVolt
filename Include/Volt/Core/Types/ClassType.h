//
// Created by bohdan on 21.06.26.
//

#ifndef CVOLT_CLASSTYPE_H
#define CVOLT_CLASSTYPE_H

#include "DataType.h"
#include "Volt/Core/TypeDefs/FunctionDefs.h"
#include "Volt/ADT/Array.h"
#include "Volt/Core/Functions/FunctionCallee.h"
#include "Volt/Core/Functions/FunctionTable.h"

namespace Volt
{
    struct Field
    {
        llvm::StringRef Name;
        QualType Type;
        size_t Offset = 0;

        Field(llvm::StringRef Name, QualType Type)
            : Name(Name), Type(Type) {}
    };

    class ClassType : public DataType
    {
        GENERATED_BODY(ClassType, DataType)
    public:
        llvm::StringRef Name;
        Array<Field> Fields;
        FunctionTable Methods;
        FuncOverloadVector Constructors;
        mutable size_t Size = 0;
        mutable size_t Alignment = 0;

    public:
        ClassType(llvm::StringRef Name, Array<Field> Fields)
            : DataType(TypeCategory::Class), Name(Name),
            Fields(std::move(Fields)) { ComputeLayout(); }

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override;
        int GetRank() const override { return -1; }
        std::string ToString() const override { return Name.str(); }
        size_t GetSize() const override;
        size_t GetAlignment() const override;
        std::string GetIRName() const override { return std::to_string(Name.size()) + Name.str(); }

        void ComputeLayout() const;

        bool CastTo(DataType *To, bool Explicit) const override { return this == To; }

        size_t GetFieldIndex(llvm::StringRef Name);

        void AddMethod(llvm::StringRef Name, ArgsVector<QualType> Params, FunctionCallee* Callee)
        {
            Methods.AddFunction(Name, std::move(Params), Callee);
        }

        void AddConstructor(ArgsVector<QualType> Params, FunctionCallee* Callee)
        {
            Constructors.emplace_back(std::move(Params), Callee);
        }

    private:
        static size_t AlignUp(size_t Offset, size_t Align)
        {
            return (Offset + Align - 1) & ~(Align - 1);
        }
    };
}

#endif //CVOLT_CLASSTYPE_H
