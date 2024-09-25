#pragma once

#include <algorithm>
#include <vector>
#include <span>
#include <array>
#include <format>
#include <stdexcept>
#include <memory>
#include <utility>
#include <span>

#include <cstdint>

#include "Crunch/Log.hpp"
#include "Crunch/Assert.hpp"

namespace Cr
{

using I8 =  int8_t;
using I16 = int16_t;
using I32 = int32_t;
using I64 = int64_t;

using U8 =  uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

using F32 = float;
using F64 = double;

template<typename F>
class Defer
{
    public:
        constexpr Defer(F&& callback) : m_function(std::forward<F>(callback)) {};

        Defer(const Defer& other) = delete;
        Defer& operator = (const F&& callback) = delete;

        Defer(Defer&& other) = delete;
        Defer& operator = (Defer&& other) = delete;

        constexpr ~Defer() { m_function(); }

    private:
        const F m_function;
};

#define CR_CONCAT_IMPL(a, b) a ## b
#define CR_CONCAT(a, b) CR_CONCAT_IMPL(a, b)

#define CR_DEFER Defer CR_CONCAT(DEFER_, __LINE__) = [&]()

template<typename T, typename D = std::default_delete<T>>
using Unique = std::unique_ptr<T, D>;

template<typename T, typename... Args>
Unique<T> create_unique(Args&& ... arguments) { return std::make_unique<T>(std::forward<Args>(arguments)...); }

} // namespace Cr

