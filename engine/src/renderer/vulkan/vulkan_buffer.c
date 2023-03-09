#include "vulkan_buffer.h"

#include "vulkan_device.h"
#include "vulkan_command_buffer.h"
#include "vulkan_utils.h"

#include "core/memory.h"
#include "core/log.h"

b8 vulkan_buffer_create(
    vulkan_context* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memory_property_flags,
    b8 bind_on_create,
    vulkan_buffer* out_buffer
) {
    memory_zero(out_buffer, sizeof(vulkan_buffer));
    out_buffer->total_size = size;
    out_buffer->usage = usage;
    out_buffer->memory_property_flags = memory_property_flags;

    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;  // NOTE: only used in one queue

    VK_CHECK(vkCreateBuffer(
        context->device.logical_device,
        &buffer_info,
        context->allocator,
        &out_buffer->handle
    ));

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(
        context->device.logical_device, out_buffer->handle, &requirements
    );
    out_buffer->memory_index = context->find_memory_index(
        requirements.memoryTypeBits, out_buffer->memory_property_flags
    );
    if (out_buffer->memory_index == -1) {
        OKO_ERROR("Failed to find memory index for buffer");
        return false;
    }

    // allocate memory info
    VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = (u32)out_buffer->memory_index;

    // allocate the memory
    VkResult result = vkAllocateMemory(
        context->device.logical_device,
        &alloc_info,
        context->allocator,
        &out_buffer->memory
    );
    if (result != VK_SUCCESS) {
        OKO_ERROR("Failed to allocate memory for buffer");
        return false;
    }

    if (bind_on_create) {
        vulkan_buffer_bind(context, out_buffer, 0);
    }

    return true;
}

void vulkan_buffer_destroy(vulkan_context* context, vulkan_buffer* buffer) {
    if (buffer->memory) {
        vkFreeMemory(
            context->device.logical_device, buffer->memory, context->allocator
        );
        buffer->memory = 0;
    }

    if (buffer->handle) {
        vkDestroyBuffer(
            context->device.logical_device, buffer->handle, context->allocator
        );
        buffer->handle = 0;
    }

    buffer->total_size = 0;
    buffer->usage = 0;
    buffer->is_locked = false;
}

b8 vulkan_buffer_resize(
    vulkan_context* context,
    u64 new_size,
    vulkan_buffer* buffer,
    VkQueue queue,
    VkCommandPool pool
) {
    // create new buffer
    VkBufferCreateInfo buffer_info = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    buffer_info.size = new_size;
    buffer_info.usage = buffer->usage;
    buffer_info.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;  // NOTE: only used in one queue

    VkBuffer new_buffer;
    VK_CHECK(vkCreateBuffer(
        context->device.logical_device,
        &buffer_info,
        context->allocator,
        &new_buffer
    ));

    // gather memory requirements
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(
        context->device.logical_device, new_buffer, &requirements
    );

    // allocate memory info
    VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = requirements.size;
    alloc_info.memoryTypeIndex = (u32)buffer->memory_index;

    // allocate the memory
    VkDeviceMemory new_memory;
    VkResult result = vkAllocateMemory(
        context->device.logical_device,
        &alloc_info,
        context->allocator,
        &new_memory
    );
    if (result != VK_SUCCESS) {
        OKO_ERROR("Failed to allocate memory for buffer");
        return false;
    }

    // bind the new buffer's memory
    VK_CHECK(vkBindBufferMemory(
        context->device.logical_device, new_buffer, new_memory, 0
    ));

    // copy over the data
    vulkan_buffer_copy_to(
        context,
        pool,
        0,
        queue,
        buffer->handle,
        0,
        new_buffer,
        0,
        buffer->total_size
    );

    // make sure anything potentially using these is finished
    vkDeviceWaitIdle(context->device.logical_device);

    // destroy the old buffer
    if (buffer->memory) {
        vkFreeMemory(
            context->device.logical_device, buffer->memory, context->allocator
        );
        buffer->memory = 0;
    }
    if (buffer->handle) {
        vkDestroyBuffer(
            context->device.logical_device, buffer->handle, context->allocator
        );
        buffer->handle = 0;
    }

    // set new properties
    buffer->total_size = new_size;
    buffer->memory = new_memory;
    buffer->handle = new_buffer;

    return true;
}

void vulkan_buffer_bind(
    vulkan_context* context, vulkan_buffer* buffer, u64 offset
) {
    VK_CHECK(vkBindBufferMemory(
        context->device.logical_device, buffer->handle, buffer->memory, offset
    ));
}

void* vulkan_buffer_lock_memory(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    u32 flags
) {
    void* data;
    VK_CHECK(vkMapMemory(
        context->device.logical_device,
        buffer->memory,
        offset,
        size,
        flags,
        &data
    ));
    return data;
}

void vulkan_buffer_unlock_memory(
    vulkan_context* context, vulkan_buffer* buffer
) {
    vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void vulkan_buffer_load_data(
    vulkan_context* context,
    vulkan_buffer* buffer,
    u64 offset,
    u64 size,
    u32 flags,
    const void* data
) {
    void* data_ptr;
    VK_CHECK(vkMapMemory(
        context->device.logical_device,
        buffer->memory,
        offset,
        size,
        flags,
        &data_ptr
    ));
    memory_copy(data_ptr, data, size);
    vkUnmapMemory(context->device.logical_device, buffer->memory);
}

void vulkan_buffer_copy_to(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size
) {
    vkQueueWaitIdle(queue);

    // create one time use command buffer
    vulkan_command_buffer temp_command_buffer;
    vulkan_command_buffer_allocate_and_begin_single_use(
        context, pool, &temp_command_buffer
    );

    // prepare the copy command and add it to the command buffer
    VkBufferCopy copy_region;
    copy_region.srcOffset = source_offset;
    copy_region.dstOffset = dest_offset;
    copy_region.size = size;

    vkCmdCopyBuffer(temp_command_buffer.handle, source, dest, 1, &copy_region);

    // submit the buffer and wait to complete
    vulkan_command_buffer_end_single_use(
        context, pool, &temp_command_buffer, queue
    );
}