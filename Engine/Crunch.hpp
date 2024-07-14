#pragma once

#include <algorithm>
#include <vector>
#include <array>
#include <format>
#include <stdexcept>
#include <memory>
#include <utility>

#include <cstdint>

using i8 =  int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 =  uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

#define CR_TERM_BLACK   "\x1B[31m"
#define CR_TERM_RED     "\x1B[31m"
#define CR_TERM_GREEN   "\x1B[32m"
#define CR_TERM_YELLOW  "\x1B[33m"
#define CR_TERM_BLUE    "\x1B[34m"
#define CR_TERM_MAGENTA "\x1B[35m"
#define CR_TERM_CYAN    "\x1B[36m"
#define CR_TERM_WHITE   "\x1B[37m"
#define CR_TERM_DEFAULT "\x1B[39m"

#define CR_TERM_RESET   "\x1B[0m"

#define CR_ASSERT_THROW(COND, ...) do {                                                   \
    if ((COND) == false) {                                                                \
        throw std::runtime_error(CR_TERM_RED + std::format(__VA_ARGS__) + CR_TERM_RESET); \
    }                                                                                     \
} while(0)

#define CR_FLOG(FD, COLOR, FORMAT, ...) do { std::fprintf(FD, COLOR FORMAT CR_TERM_RESET __VA_OPT__(,)__VA_ARGS__); } while(0)

#define CR_INFO(FORMAT,  ...) CR_FLOG(stdout, CR_TERM_DEFAULT, FORMAT, __VA_ARGS__)
#define CR_WARN(FORMAT,  ...) CR_FLOG(stderr, CR_TERM_YELLOW,  FORMAT, __VA_ARGS__)
#define CR_ERROR(FORMAT, ...) CR_FLOG(stderr, CR_TERM_RED,     FORMAT, __VA_ARGS__)

namespace Cr
{

template<typename F>
class Defer
{
    public:
        constexpr Defer(F&& callback) : m_function(std::forward<F>(callback)) {};

        Defer(const Defer& other) = delete;
        Defer& operator = (const F&& callback) = delete;

        Defer(Defer&& other) = delete;
        Defer& operator = (Defer&& other) = delete;

        ~Defer() { m_function(); }

    private:
        const F m_function;
};

template<typename T, typename D = std::default_delete<T>>
using Unique = std::unique_ptr<T, D>;

template<typename T, typename... Args>
Unique<T> create_unique(Args&& ... arguments) { return std::make_unique<T>(std::forward(arguments...)); }

} // namespace Cr

