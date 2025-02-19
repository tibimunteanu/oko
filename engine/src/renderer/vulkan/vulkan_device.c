#include "renderer/vulkan/vulkan_device.h"

#include "core/log.h"
#include "core/memory.h"

#include "containers/string.h"
#include "containers/darray.h"

typedef struct vulkan_physical_device_requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    // darray
    const char** device_extension_names;
    b8 sampler_anisotropy;
    b8 discrete_gpu;
} vulkan_physical_device_requirements;

typedef struct vulkan_physical_device_queue_family_info {
    u32 graphics_family_index;
    u32 present_family_index;
    u32 compute_family_index;
    u32 transfer_family_index;
} vulkan_physical_device_queue_family_info;

// PRIVATE
b8 physical_device_meets_requirements(
    VkPhysicalDevice device,
    VkSurfaceKHR surface,
    const VkPhysicalDeviceProperties* properties,
    const VkPhysicalDeviceFeatures* features,
    const vulkan_physical_device_requirements* requirements,
    vulkan_physical_device_queue_family_info* out_queue_info,
    vulkan_swapchain_support_info* out_swapchain_support
) {
    //
    // Evaluate device properties to determine if it meets the requirements of
    // the app
    out_queue_info->graphics_family_index = -1;
    out_queue_info->present_family_index = -1;
    out_queue_info->compute_family_index = -1;
    out_queue_info->transfer_family_index = -1;

    // Discrete GPU?
    if (requirements->discrete_gpu &&
        properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        OKO_INFO("Device is not a discrete GPU, and one is required. Skipping.")
        return false;
    }

    // TODO: VK_CHECK?
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 0);
    VkQueueFamilyProperties queue_families[32];
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, queue_families
    );

    // Look at each queue and see what operations it supports
    u8 min_transfer_score = 255;
    for (u32 i = 0; i < queue_family_count; i++) {
        u8 current_transfer_score = 0;

        // Graphics queue?
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            out_queue_info->graphics_family_index = i;
            ++current_transfer_score;
        }

        // Compute queue?
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            out_queue_info->compute_family_index = i;
            ++current_transfer_score;
        }

        // Transfer queue?
        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            // Take the index if it is the current lowest. This increases the
            // likelihood that it is a dedicated transfer queue.
            if (current_transfer_score <= min_transfer_score) {
                min_transfer_score = current_transfer_score;
                out_queue_info->transfer_family_index = i;
            }
        }

        // Present queue?
        VkBool32 supports_present = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(
            device, i, surface, &supports_present
        ));
        if (supports_present) {
            out_queue_info->present_family_index = i;
        }
    }

    if ((requirements->graphics && out_queue_info->graphics_family_index == -1
        ) ||
        (requirements->present && out_queue_info->present_family_index == -1) ||
        (requirements->compute && out_queue_info->compute_family_index == -1) ||
        (requirements->transfer && out_queue_info->transfer_family_index == -1
        )) {
        //
        OKO_INFO("Device does not meet queue requirements. Skipping.");
        return false;
    }

    // Query swapchain support.
    vulkan_device_query_swapchain_support(
        device, surface, out_swapchain_support
    );

    if (out_swapchain_support->format_count < 1 ||
        out_swapchain_support->present_mode_count < 1) {
        if (out_swapchain_support->formats) {
            memory_free(
                out_swapchain_support->formats,
                sizeof(VkSurfaceFormatKHR) *
                    out_swapchain_support->format_count,
                MEMORY_TAG_RENDERER
            );
        }
        if (out_swapchain_support->present_modes) {
            memory_free(
                out_swapchain_support->present_modes,
                sizeof(VkPresentModeKHR) *
                    out_swapchain_support->present_mode_count,
                MEMORY_TAG_RENDERER
            );
        }

        memory_zero(
            &out_swapchain_support->capabilities,
            sizeof(out_swapchain_support->capabilities)
        );

        OKO_INFO("Device required swapchain support not present. Skipping.");
        return false;
    }

    // Device extensions
    u32 required_extensions_count = 0;
    if (requirements->device_extension_names) {
        required_extensions_count =
            darray_length(requirements->device_extension_names);
    }

    if (required_extensions_count != 0) {
        u32 available_extension_count = 0;
        VkExtensionProperties* available_extensions = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(
            device, 0, &available_extension_count, 0
        ));

        if (available_extension_count == 0) {
            OKO_INFO("Device required extensions not present. Skipping.");
            return false;
        }

        available_extensions = memory_allocate(
            sizeof(VkExtensionProperties) * available_extension_count,
            MEMORY_TAG_RENDERER
        );
        VK_CHECK(vkEnumerateDeviceExtensionProperties(
            device, 0, &available_extension_count, available_extensions
        ));

        for (u32 i = 0; i < required_extensions_count; i++) {
            b8 found = false;
            for (u32 j = 0; j < available_extension_count; j++) {
                if (strings_equal(
                        requirements->device_extension_names[i],
                        available_extensions[j].extensionName
                    )) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                OKO_INFO(
                    "Device required extension not found: '%s'. Skipping.",
                    requirements->device_extension_names[i]
                );
                memory_free(
                    available_extensions,
                    sizeof(VkExtensionProperties) * available_extension_count,
                    MEMORY_TAG_RENDERER
                );
                return false;
            }
        }

        memory_free(
            available_extensions,
            sizeof(VkExtensionProperties) * available_extension_count,
            MEMORY_TAG_RENDERER
        );

        // Sampler anisotropy
        if (requirements->sampler_anisotropy && !features->samplerAnisotropy) {
            OKO_INFO("Device does not support sampler anisotropy. Skipping.");
            return false;
        }
    }

    // Device meets all requirements
    return true;
}

void print_device_info(vulkan_device* device) {
    // Print some info about the selected device
    OKO_INFO("Device name: '%s'.", device->properties.deviceName);

    // GPU info
    switch (device->properties.deviceType) {
    default:
    case VK_PHYSICAL_DEVICE_TYPE_OTHER: OKO_INFO("GPU type: Unknown"); break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        OKO_INFO("GPU type: Integrated");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        OKO_INFO("GPU type: Discrete");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        OKO_INFO("GPU type: Virtual");
        break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU: OKO_INFO("GPU type: CPU"); break;
    }

    OKO_INFO(
        "GPU Driver version: %d.%d.%d",
        VK_VERSION_MAJOR(device->properties.driverVersion),
        VK_VERSION_MINOR(device->properties.driverVersion),
        VK_VERSION_PATCH(device->properties.driverVersion)
    );

    // Vulkan API version.
    OKO_INFO(
        "Vulkan API version: %d.%d.%d",
        VK_VERSION_MAJOR(device->properties.apiVersion),
        VK_VERSION_MINOR(device->properties.apiVersion),
        VK_VERSION_PATCH(device->properties.apiVersion)
    );

    // Memory information
    OKO_INFO("Memory heaps:");
    for (u32 j = 0; j < device->memory.memoryHeapCount; j++) {
        f32 memory_size_gib =
            (((f32)device->memory.memoryHeaps[j].size) / 1024.0f / 1024.0f /
             1024.0f);
        if (device->memory.memoryHeaps[j].flags &
            VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            OKO_INFO("\tLocal: %.2f GiB", memory_size_gib);
        } else {
            OKO_INFO("\tShared: %.2f GiB", memory_size_gib);
        }
    }

    OKO_INFO("Queue families: | Graphics | Present | Compute | Transfer |");
    OKO_INFO(
        "                |        %d |       %d |       %d |        %d |",
        device->graphics_queue_index,
        device->present_queue_index,
        device->compute_queue_index,
        device->transfer_queue_index
    );
}

b8 select_physical_device(vulkan_context* context) {
    OKO_INFO("Selecting physical device...");

    // Get device count
    u32 physical_device_count = 0;
    VK_CHECK(
        vkEnumeratePhysicalDevices(context->instance, &physical_device_count, 0)
    );
    if (physical_device_count == 0) {
        OKO_FATAL("No devices which support Vulkan were found.;")
        return false;
    }

    // Get all devices info
    const u32 max_device_count = 32;
    VkPhysicalDevice physical_devices[max_device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(
        context->instance, &physical_device_count, physical_devices
    ));

    // Select the best device
    for (u32 i = 0; i < physical_device_count; i++) {
        // Query info about the device's properties, features and memory
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physical_devices[i], &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(physical_devices[i], &features);

        VkPhysicalDeviceMemoryProperties memory;
        vkGetPhysicalDeviceMemoryProperties(physical_devices[i], &memory);

        // check if device supports local/host visible combo
        b8 supports_device_local_host_visible = false;
        for (u32 i = 0; i < memory.memoryTypeCount; i++) {
            // check each memory type to see if its bit is set to 1
            if ((memory.memoryTypes[i].propertyFlags &
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) &&
                (memory.memoryTypes[i].propertyFlags &
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
                supports_device_local_host_visible = true;
                break;
            }
        }

        // TODO: These requirements should be driven by config, but for now by a
        // struct
        vulkan_physical_device_requirements requirements = {};
        requirements.graphics = true;
        requirements.present = true;
        requirements.transfer = true;
        requirements.compute = true;
        requirements.sampler_anisotropy = true;
        requirements.discrete_gpu = true;
        requirements.device_extension_names = darray_create(const char*);
        darray_push(
            requirements.device_extension_names,
            &VK_KHR_SWAPCHAIN_EXTENSION_NAME
        );

        // Check the device against the requirements
        vulkan_physical_device_queue_family_info queue_info = {};
        b8 found = physical_device_meets_requirements(
            physical_devices[i],
            context->surface,
            &properties,
            &features,
            &requirements,
            &queue_info,
            &context->device.swapchain_support
        );

        if (found) {
            // Save the physical device, queue indices and other device info
            context->device.physical_device = physical_devices[i];
            context->device.graphics_queue_index =
                queue_info.graphics_family_index;
            context->device.present_queue_index =
                queue_info.present_family_index;
            context->device.compute_queue_index =
                queue_info.compute_family_index;
            context->device.transfer_queue_index =
                queue_info.transfer_family_index;
            context->device.properties = properties;
            context->device.features = features;
            context->device.memory = memory;
            context->device.supports_device_local_host_visible =
                supports_device_local_host_visible;

            break;
        }
    }

    // Ensure a device was selected
    if (!context->device.physical_device) {
        OKO_ERROR("No physical devices were found which meet the requirements."
        );
        return false;
    }

    print_device_info(&context->device);

    return true;
}

void create_logical_device(vulkan_context* context) {
    OKO_INFO("Creating logical device...");

    // NOTE: Don not create additional queues for shared indices.
    b8 present_shares_graphics_queue = context->device.graphics_queue_index ==
                                       context->device.present_queue_index;
    b8 transfer_shares_graphics_queue = context->device.graphics_queue_index ==
                                        context->device.transfer_queue_index;
    u32 index_count = 1;
    if (!present_shares_graphics_queue) {
        index_count++;
    }
    if (!transfer_shares_graphics_queue) {
        index_count++;
    }
    u32 indices[32];
    u8 index = 0;
    indices[index++] = context->device.graphics_queue_index;
    if (!present_shares_graphics_queue) {
        indices[index++] = context->device.present_queue_index;
    }
    if (!transfer_shares_graphics_queue) {
        indices[index++] = context->device.transfer_queue_index;
    }

    VkDeviceQueueCreateInfo queue_create_infos[32];
    for (u32 i = 0; i < index_count; i++) {
        queue_create_infos[i].sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = indices[i];
        queue_create_infos[i].queueCount = 1;
        // TODO: Enable this for a future enhancement.
        // if (indices[i] == context->device.graphics_queue_index) {
        //     queue_create_infos[i].queueCount = 2;
        // }
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    // Request device features.
    // TODO: should be config driven
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;  // Request anisotropy

    VkDeviceCreateInfo device_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = index_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = 1;
    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    device_create_info.ppEnabledExtensionNames = &extension_names;

    // Deprecated and ignored, so pass nothing.
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;

    // create the device
    VK_CHECK(vkCreateDevice(
        context->device.physical_device,
        &device_create_info,
        context->allocator,
        &context->device.logical_device
    ));

    OKO_INFO("Logical device created.");

    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.graphics_queue_index,
        0,
        &context->device.graphics_queue
    );

    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.present_queue_index,
        0,
        &context->device.present_queue
    );

    vkGetDeviceQueue(
        context->device.logical_device,
        context->device.transfer_queue_index,
        0,
        &context->device.transfer_queue
    );

    // TODO: get compute queue

    OKO_INFO("Queues obtained.");

    // Create command pool for graphics queue
    VkCommandPoolCreateInfo pool_create_info = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_create_info.queueFamilyIndex = context->device.graphics_queue_index;
    pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(
        context->device.logical_device,
        &pool_create_info,
        context->allocator,
        &context->device.graphics_command_pool
    ));

    OKO_INFO("Graphics command pool created.");
}

// PUBLIC
b8 vulkan_device_create(vulkan_context* context) {
    if (!select_physical_device(context)) {
        return false;
    }

    create_logical_device(context);

    return true;
}

void vulkan_device_destroy(vulkan_context* context) {
    // Unset queues
    context->device.graphics_queue = 0;
    context->device.present_queue = 0;
    context->device.transfer_queue = 0;
    context->device.compute_queue = 0;

    context->device.graphics_queue_index = -1;
    context->device.transfer_queue_index = -1;
    context->device.present_queue_index = -1;
    context->device.compute_queue_index = -1;

    // Destroy command pools
    OKO_INFO("Destroying command pools...");
    vkDestroyCommandPool(
        context->device.logical_device,
        context->device.graphics_command_pool,
        context->allocator
    );

    // Destroy logical device
    OKO_INFO("Destroying logical device...");
    if (context->device.logical_device) {
        vkDestroyDevice(context->device.logical_device, context->allocator);
        context->device.logical_device = 0;
    }

    // Physical devices are not destroyed.
    OKO_INFO("Releasing physical device resources...");
    context->device.physical_device = 0;

    if (context->device.swapchain_support.formats) {
        memory_free(
            context->device.swapchain_support.formats,
            sizeof(VkSurfaceFormatKHR) *
                context->device.swapchain_support.format_count,
            MEMORY_TAG_RENDERER
        );
        context->device.swapchain_support.formats = 0;
        context->device.swapchain_support.format_count = 0;
    }

    if (context->device.swapchain_support.present_modes) {
        memory_free(
            context->device.swapchain_support.present_modes,
            sizeof(VkPresentModeKHR) *
                context->device.swapchain_support.present_mode_count,
            MEMORY_TAG_RENDERER
        );
        context->device.swapchain_support.present_modes = 0;
        context->device.swapchain_support.present_mode_count = 0;
    }

    memory_zero(
        &context->device.swapchain_support.capabilities,
        sizeof(context->device.swapchain_support.capabilities)
    );
}

void vulkan_device_query_swapchain_support(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface,
    vulkan_swapchain_support_info* out_support_info
) {
    //
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physical_device, surface, &out_support_info->capabilities
    ));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        physical_device, surface, &out_support_info->format_count, 0
    ));
    if (out_support_info->format_count != 0) {
        if (!out_support_info->formats) {
            out_support_info->formats = memory_allocate(
                sizeof(VkSurfaceFormatKHR) * out_support_info->format_count,
                MEMORY_TAG_RENDERER
            );
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &out_support_info->format_count,
            out_support_info->formats
        ));
    }

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
        physical_device, surface, &out_support_info->present_mode_count, 0
    ));
    if (out_support_info->present_mode_count != 0) {
        if (!out_support_info->present_modes) {
            out_support_info->present_modes = memory_allocate(
                sizeof(VkPresentModeKHR) * out_support_info->present_mode_count,
                MEMORY_TAG_RENDERER
            );
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &out_support_info->present_mode_count,
            out_support_info->present_modes
        ));
    }
}

b8 vulkan_device_detect_depth_format(vulkan_device* device) {
    // Format candidates
    const u32 candidate_count = 3;
    VkFormat candidates[3] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT};

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u32 i = 0; i < candidate_count; i++) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(
            device->physical_device, candidates[i], &properties
        );

        if (((properties.linearTilingFeatures & flags) == flags) ||
            ((properties.optimalTilingFeatures & flags) == flags)) {
            device->depth_format = candidates[i];
            return true;
        }
    }
    return false;
}