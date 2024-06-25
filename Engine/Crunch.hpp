#pragma once

#include <cstdint>
#include <stdexcept>

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

#define CR_ASSERT_THROW(COND, MSG) do {                               \
    if ((COND) == false) {                                            \
        throw std::runtime_error(CR_TERM_RED MSG CR_TERM_RESET "\n"); \
    }                                                                 \
} while(0)

#define CR_FLOG(FD, COLOR,  ...) do { std::fputs(COLOR, FD); std::fprintf(FD, __VA_ARGS__); std::fputs(CR_TERM_RESET"\n", FD); } while(0)

#define CR_INFO(...)   CR_FLOG(stdout, CR_TERM_DEFAULT, __VA_ARGS__)
#define CR_WARN(...)   CR_FLOG(stderr, CR_TERM_YELLOW,  __VA_ARGS__)
#define CR_ERROR(...)  CR_FLOG(stderr, CR_TERM_RED,     __VA_ARGS__)

#define CR_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(*arr))

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
