//
// Created by bohdan on 09.06.26.
//

#ifndef CVOLT_ERRORHANDLING_H
#define CVOLT_ERRORHANDLING_H

#include <cstdio>

#ifdef _MSC_VER
    #define VOLT_FUNC __FUNCSIG__
#else
    #define VOLT_FUNC __PRETTY_FUNCTION__
#endif

[[noreturn]] inline void VoltUnreachableImpl(
    const char* Msg, const char* File, unsigned Line, const char* Func)
{
    std::fprintf(stderr, "UNREACHABLE: %s\n  at %s:%d in %s\n",
        Msg, File, Line, Func);
    std::abort();
}

#define VoltUnreachable(Msg) VoltUnreachableImpl(Msg, __FILE__, __LINE__, VOLT_FUNC)
#define VoltAssert(Expr) (static_cast<bool>(Expr) ? void(0) : VoltUnreachable(#Expr))

#endif //CVOLT_ERRORHANDLING_H
