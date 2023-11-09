#pragma once

#include <vulkan/vulkan_core.h>

#include "Crunch.hpp"

#define VK_ASSERT_RETURN(RESULT, MSG) { VkResult _error = RESULT; if (_error < 0) return _error; }
#define VK_ASSERT_THROW(RESULT, MSG) CR_ASSERT_THROW((RESULT) < 0, MSG)
