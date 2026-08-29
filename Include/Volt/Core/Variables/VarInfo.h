//
// Created by bohdan on 8/28/26.
//

#ifndef CVOLT_VARINFO_H
#define CVOLT_VARINFO_H

namespace Volt
{
    class ExprAddress;
    class IRValue;

    struct VarInfo
    {
        ExprAddress* SemaValue;
        IRValue* CodeGenValue;

        VarInfo(ExprAddress* SemaValue)
            : SemaValue(SemaValue), CodeGenValue(nullptr) {}
    };
}

#endif //CVOLT_VARINFO_H
