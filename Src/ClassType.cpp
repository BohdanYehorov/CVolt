//
// Created by bohdan on 21.06.26.
//

#include "Volt/Core/Types/ClassType.h"
#include "Volt/Core/TypeDefs/TypeDefs.h"

namespace Volt
{
    llvm::Type *ClassType::ToLLVMType(llvm::LLVMContext &Context) const
    {
        SmallVec4<llvm::Type*> Types;
        Types.reserve(Fields.Length());
        for (auto Field : Fields)
            Types.push_back(Field.Type->ToLLVMType(Context));

        return llvm::StructType::create(Context, Types, Name);
    }

    size_t ClassType::GetFieldIndex(const std::string &Name)
    {
        for (size_t i = 0; i < Fields.Length(); i++)
        {
            if (Fields[i].Name == Name)
                return i;
        }

        return Fields.Length();
    }
}
