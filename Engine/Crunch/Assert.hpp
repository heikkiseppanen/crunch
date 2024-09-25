#pragma once

#include "Crunch/Log.hpp"

#include <exception>
#include <format>

#ifdef _MSC_VER
#include <intrin.h>
#define CR_RBEAK() __debugbreak()
#elif __linux__
#include <signal.h>
#define CR_BREAK() raise(SIGTRAP)
#endif

#define CR_ASSERT(COND, FMT, ...) do {                                                                                 \
    if ((COND) == false) [[unlikely]]                                                                                  \
    {                                                                                                                  \
        CR_ERROR("{}:{} " FMT, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__);                                          \
        CR_BREAK();                                                                                                    \
    }                                                                                                                  \
} while(0)

#define CR_ASSERT_THROW(COND, ...) do {                                                                                \
    if ((COND) == false) [[unlikely]]                                                                                  \
    {                                                                                                                  \
        throw std::runtime_error(CR_TERM_RED + std::format(__VA_ARGS__) + CR_TERM_RESET + '\n');                       \
    }                                                                                                                  \
} while(0)
