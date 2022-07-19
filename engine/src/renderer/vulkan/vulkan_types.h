#pragma once

#include "defines.h"
#include "core/assert.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr)                  \
    {                                   \
        OKO_ASSERT(expr == VK_SUCCESS); \
    }

typedef struct vulkan_context {
    VkInstance instance;
    VkAllocationCallbacks* allocator;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
} vulkan_context;