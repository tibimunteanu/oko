#pragma once

#include "defines.h"
#include "core/assert.h"

#include "renderer/renderer_types.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(expr) \
  { OKO_ASSERT(expr == VK_SUCCESS); }

typedef struct vulkan_buffer {
    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlags usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 memory_property_flags;
} vulkan_buffer;

typedef struct vulkan_swapchain_support_info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vulkan_swapchain_support_info;

typedef struct vulkan_device {
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 compute_queue_index;
    i32 transfer_queue_index;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;
    VkQueue compute_queue;
    VkCommandPool graphics_command_pool;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    VkFormat depth_format;
} vulkan_device;

typedef struct vulkan_image {
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

typedef enum vulkan_render_pass_state {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkan_render_pass_state;

typedef struct vulkan_renderpass {
    VkRenderPass handle;
    f32 x, y, w, h;
    f32 r, g, b, a;
    f32 depth;
    u32 stencil;
} vulkan_renderpass;

typedef struct vulkan_framebuffer {
    VkFramebuffer handle;
    u32 attachment_count;
    VkImageView* attachments;
    vulkan_renderpass* renderpass;
} vulkan_framebuffer;

typedef struct vulkan_swapchain {
    VkSurfaceFormatKHR image_format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 image_count;
    VkImage* images;
    VkImageView* views;
    vulkan_image depth_attachment;
    vulkan_framebuffer* framebuffers;  // darray
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state {
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED,
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer {
    VkCommandBuffer handle;
    vulkan_command_buffer_state state;
} vulkan_command_buffer;

typedef struct vulkan_fence {
    VkFence handle;
    b8 is_signaled;
} vulkan_fence;

typedef struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
} vulkan_pipeline;

#define OBJECT_SHADER_STAGE_COUNT             2
#define VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_OBJECT_MAX_OBJECT_COUNT        1024

typedef struct vulkan_descriptor_state {
    // one per frame
    u32 generations[3];
} vulkan_descriptor_state;

typedef struct vulkan_object_shader_object_state {
    // per frame
    VkDescriptorSet descriptor_sets[3];

    // per descriptor
    vulkan_descriptor_state
        descriptor_states[VULKAN_OBJECT_SHADER_DESCRIPTOR_COUNT];
} vulkan_object_shader_object_state;

typedef struct vulkan_object_shader {
    vulkan_shader_stage stages[OBJECT_SHADER_STAGE_COUNT];
    // descriptors
    VkDescriptorPool global_descriptor_pool;
    VkDescriptorSetLayout global_descriptor_set_layout;
    // one per frame - max 3 for triple buffering
    VkDescriptorSet global_descriptor_sets[3];
    global_uniform_object global_ubo;
    vulkan_buffer global_uniform_buffer;

    VkDescriptorPool object_descriptor_pool;
    VkDescriptorSetLayout object_descriptor_set_layout;
    // object uniform buffers
    vulkan_buffer object_uniform_buffer;
    // TODO: manage a free list of some kind here instead
    u32 object_uniform_buffer_index;

    // TODO: make dynamic
    vulkan_object_shader_object_state
        object_states[VULKAN_OBJECT_MAX_OBJECT_COUNT];

    // pointers to default textures
    texture* default_diffuse;

    // pipeline
    vulkan_pipeline pipeline;
} vulkan_object_shader;

typedef struct vulkan_context {
    f32 frame_delta_time;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u64 framebuffer_size_generation;
    u64 framebuffer_size_last_generation;
    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

#if defined(_DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    vulkan_device device;
    vulkan_swapchain swapchain;
    vulkan_renderpass main_renderpass;
    vulkan_buffer object_vertex_buffer;
    vulkan_buffer object_index_buffer;
    vulkan_command_buffer* graphics_command_buffers;  // darray
    VkSemaphore* image_available_semaphores;          // darray
    VkSemaphore* queue_complete_semaphores;           // darray
    u32 in_flight_fence_count;
    vulkan_fence* in_flight_fences;
    vulkan_fence** images_in_flight;  // Holds pointers to fences which exist
                                      // and are owned elsewhere
    u32 image_index;
    u32 current_frame;
    b8 recreating_swapchain;

    vulkan_object_shader object_shader;

    u64 geometry_vertex_offset;
    u64 geometry_index_offset;

    i32 (*find_memory_index)(u32 type_filter, u32 property_flags);
} vulkan_context;

typedef struct vulkan_texture_data {
    vulkan_image image;
    VkSampler sampler;
} vulkan_texture_data;