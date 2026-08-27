//
// Created by bohdan on 8/27/26.
//

#ifndef CVOLT_POINTERINTPAIR_H
#define CVOLT_POINTERINTPAIR_H

#include <cstddef>
#include "Volt/Core/TypeDefs/IntTypeDefs.h"
#include "Volt/Support/ErrorHandling.h"

namespace Volt
{
    template <typename T, size_t Align = alignof(UIntPtrTy), typename IntTy = UInt32>
    class PointerIntPair
    {
        static_assert(alignof(T) >= Align);
    public:
        using PointerType = T*;
        using IntType = IntTy;

    private:
        UIntPtrTy Value = 0;

    public:
        PointerIntPair() = default;

        PointerIntPair(PointerType Ptr, IntType Int)
        {
            SetPointerAndInt(Ptr, Int);
        }

        void SetPointerAndInt(PointerType Ptr, IntType Int)
        {
            UIntPtrTy PtrVal = reinterpret_cast<UIntPtrTy>(Ptr);
            VoltAssert((PtrVal & (Align - 1)) == 0 && "Pointer is not aligned");
            VoltAssert(Int < Align && "Cannot write value grater or equal align");
            Value = PtrVal | Int;
        }

        void SetPointer(PointerType Ptr)
        {
            UIntPtrTy PtrVal = reinterpret_cast<UIntPtrTy>(Ptr);
            VoltAssert((PtrVal & Align - 1) == 0 && "Pointer is not aligned");
            Value = PtrVal | (Value & Align - 1);
        }

        void SetInt(IntType Int)
        {
            VoltAssert(Int < Align && "Cannot write value grater or equal align");
            Value = Int | (Value & ~(Align - 1));
        }

        [[nodiscard]] PointerType GetPointer() const
        {
            return reinterpret_cast<PointerType>(Value & Align - 1);
        }

        [[nodiscard]] IntType GetInt() const
        {
            return Value & ~(Align - 1);
        }
    };
}
#endif //CVOLT_POINTERINTPAIR_H
