#pragma once

#include <vulkan/vulkan_core.h>

#include "Crunch.hpp"

#define VK_ASSERT_RETURN(RESULT, MSG) { VkResult = RESULT; if (RESULT < 0) return RESULT; }
#define VK_ASSERT_THROW(RESULT, MSG) CR_ASSERT_THROW((RESULT) < 0, MSG)
