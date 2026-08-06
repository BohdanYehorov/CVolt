//
// Created by bohdan on 23.05.26.
//

#ifndef CVOLT_ALIGNEDSTORAGE_H
#define CVOLT_ALIGNEDSTORAGE_H

#include <algorithm>

namespace Volt
{
    template <typename ...Ts>
    struct AlignedStorage
    {
        static constexpr size_t Align = std::max({alignof(Ts)...});
        alignas(Align) char Buffer[std::max({sizeof(Ts)...})];

        template <typename T>
        T& As()
        {
            static_assert((std::same_as<T, Ts> || ...));
            return *std::launder(reinterpret_cast<T*>(Buffer));
        }

        template <typename T>
        const T& As() const
        {
            static_assert((std::same_as<T, Ts> || ...));
            return *std::launder(reinterpret_cast<const T*>(Buffer));
        }

        template <typename T>
        void Set(const T& Value) { As<T>() = Value; }

        template <typename T, typename ...ArgsTy>
        void Construct(ArgsTy&&... Args)
        {
            new (Buffer) T(std::forward<ArgsTy>(Args)...);
        }

        template <typename T>
        void Destruct() { As<T>()->~T(); }
    };
}

#endif //CVOLT_ALIGNEDSTORAGE_H
