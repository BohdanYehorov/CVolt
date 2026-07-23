//
// Created by bohdan on 20.07.26.
//

#ifndef CVOLT_CLASSINST_H
#define CVOLT_CLASSINST_H

#include "ClassType.h"
#include "TypeConv.h"
#include "Volt/Core/CompilationContext/CompilationContext.h"

namespace Volt
{
    class ClassInstBase
    {
    protected:
        ClassType* Type = nullptr;
        char* Data = nullptr;
        CompilationContext& CContext;

        ClassInstBase(ClassType* Type, CompilationContext& CContext)
            : Type(Type), CContext(CContext) { }

        ClassInstBase(const std::string& Name, const Array<Field>& Fields,
            CompilationContext& CContext) : CContext(CContext)
        {
            Type = CContext.CreateClassType(Name, Fields);
        }

    public:
        [[nodiscard]] ClassType* GetType() const { return Type; }
        [[nodiscard]] void *GetData() const { return Data; }

        [[nodiscard]] void *GetField(llvm::StringRef FieldName, size_t Size) const;
        template <typename T>
        T& GetField(llvm::StringRef FieldName);

        [[nodiscard]] class ClassInstView GetField(llvm::StringRef FieldName) const;
    };

    class ClassInst : public ClassInstBase
    {
    public:
        ClassInst(ClassType* Type, CompilationContext& CContext)
            : ClassInstBase(Type, CContext)
        {
            Data = static_cast<char*>(operator new(Type->GetSize()));
        }

        ClassInst(const std::string& Name, const Array<Field>& Fields, CompilationContext& CContext)
            : ClassInstBase(Name, Fields, CContext)
        {
            Data = static_cast<char*>(operator new(Type->GetSize()));
        }

        ClassInst(const ClassInst& Other);
        ClassInst& operator=(const ClassInst& Other);

        ClassInst(ClassInst&& Other) noexcept;
        ClassInst& operator=(ClassInst&& Other) noexcept;

        ~ClassInst() { operator delete(Data); }
    };

    class ClassInstView : public ClassInstBase
    {
    public:
        ClassInstView(ClassType* Type, char* InData, CompilationContext& CContext)
            : ClassInstBase(Type, CContext) { Data = InData; }
    };

    template<typename T>
    T& ClassInstBase::GetField(llvm::StringRef FieldName)
    {
        size_t FieldIndex = Type->GetFieldIndex(FieldName.str());
        VoltAssert(FieldIndex != Type->Fields.Length());
        const Field& F = Type->Fields[FieldIndex];
        QualType Ty = TypeConv::GetDataType<T>(CContext);
        if (Ty != F.Type)
            VoltUnreachableFmt("Cannot get field with type '{}' as '{}'", F.Type.ToString(), Ty.ToString());
        return *reinterpret_cast<T*>(Data + F.Offset);
    }
}

#endif //CVOLT_CLASSINST_H
