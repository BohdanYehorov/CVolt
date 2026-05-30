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
    };
}

#endif //CVOLT_ALIGNEDSTORAGE_H
