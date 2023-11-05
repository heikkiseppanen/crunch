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
	if (COND) {                                                       \
		throw std::runtime_error(CR_TERM_RED MSG CR_TERM_RESET "\n"); \
	}                                                                 \
} while(0);

#define CR_LOG(FD, COLOR, MSG) std::fprintf(FD, COLOR "%s" CR_TERM_RESET "\n", MSG);

#define CR_INFO(MSG)  CR_LOG(stdout, CR_TERM_DEFAULT, MSG)
#define CR_WARN(MSG)  CR_LOG(stderr, CR_TERM_YELLOW, MSG)
#define CR_ERROR(MSG) CR_LOG(stderr, CR_TERM_RED, MSG)

#define CR_ARR_SIZE(arr) (sizeof(arr) / sizeof(*arr))
