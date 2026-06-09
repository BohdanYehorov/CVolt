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

#ifdef NDEBUG
#define VoltUnreachable(msg) __builtin_unreachable()
#else
#define VoltUnreachable(msg) (void)(std::fprintf(stderr, "UNREACHABLE: %s\n  at %s:%d in %s\n", \
msg, __FILE__, __LINE__, VOLT_FUNC), \
std::abort())
#endif

#endif //CVOLT_ERRORHANDLING_H
