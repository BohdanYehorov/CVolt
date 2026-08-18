//
// Created by bohdan on 21.06.26.
//

#ifndef CVOLT_CLASSTYPE_H
#define CVOLT_CLASSTYPE_H

#include "DataType.h"
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
    private:
        llvm::StringRef Name;
        Array<Field> Fields;
        MethodTable Methods;
        FuncOverloadTable Constructors;
        mutable size_t Size = 0;
        mutable size_t Alignment = 0;

        bool ClassInitialized;

    public:
        ClassType(llvm::StringRef Name, Array<Field> Fields)
            : DataType(TypeCategory::Class), Name(Name),
            Fields(std::move(Fields)), ClassInitialized(true) { ComputeLayout(); }

        ClassType(llvm::StringRef Name)
            : DataType(TypeCategory::Class),
            Name(Name), ClassInitialized(false) {}

        llvm::Type* ToLLVMType(llvm::LLVMContext &Context) const override;
        int GetRank() const override { return -1; }
        std::string ToString() const override { return Name.str(); }
        size_t GetSize() const override;
        size_t GetAlignment() const override;
        std::string GetIRName() const override { return std::to_string(Name.size()) + Name.str(); }

        void ComputeLayout() const;
        bool CastTo(DataType *To, bool Explicit) const override { return this == To; }

        [[nodiscard]] size_t GetFieldIndex(llvm::StringRef Name);
        [[nodiscard]] const Field& GetField(size_t Index) const { return Fields[Index]; }
        [[nodiscard]] size_t GetFieldsCount() const { return Fields.Length(); }

        [[nodiscard]] const MethodTable& GetMethods() const { return Methods; }

        [[nodiscard]] llvm::StringRef GetName() { return Name; }
        [[nodiscard]] const FunctionOverload* FindBestMethodOverload(
            llvm::StringRef Name, llvm::ArrayRef<QualType> Params) const
        {
            return Methods.FindBestFunctionOverload(Name, Params);
        }

        [[nodiscard]] const FunctionOverload* FindBestConstructorOverload(
            llvm::ArrayRef<QualType> Params) const
        {
            return Constructors.FindBestOverload(Params);
        }

        void AddField(llvm::StringRef Name, QualType Type)
        {
            VoltAssert(!ClassInitialized && "Cannot add field to initialized class");
            Fields.Emplace(Name, Type);
        }

        void AddMethod(llvm::StringRef Name, ArgsVector<QualType> Params, FunctionCallee* Callee)
        {
            Methods.AddFunction(Name, std::move(Params), Callee, this);
        }

        void AddConstructor(ArgsVector<QualType> Params, FunctionCallee* Callee)
        {
            Constructors.AddOverload(std::move(Params), Callee);
        }

        void FinishInitializing()
        {
            ClassInitialized = true;
            ComputeLayout();
        }

    private:
        static size_t AlignUp(size_t Offset, size_t Align)
        {
            return (Offset + Align - 1) & ~(Align - 1);
        }
    };
}

#endif //CVOLT_CLASSTYPE_H
