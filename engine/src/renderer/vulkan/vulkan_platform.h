#pragma once

#include "defines.h"

// Appends the names of required extensions for this platform to the
// names_darray.
void platform_push_vulkan_required_extension_names(const char*** names_darray);

b8 platform_create_vulkan_surface(struct vulkan_context* context);