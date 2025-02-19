#pragma once

#include "renderer/vulkan/vulkan_types.h"

void vulkan_image_create(
    vulkan_context* context,
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    vulkan_image* out_image
);

void vulkan_image_view_create(
    vulkan_context* context,
    VkFormat format,
    vulkan_image* image,
    VkImageAspectFlags aspect_flags
);

void vulkan_image_transition_layout(
    vulkan_context* context,
    vulkan_command_buffer* command_buffer,
    vulkan_image* image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout
);

void vulkan_image_copy_from_buffer(
    vulkan_context* context,
    vulkan_image* image,
    VkBuffer buffer,
    vulkan_command_buffer* command_buffer
);

void vulkan_image_destroy(vulkan_context* context, vulkan_image* image);