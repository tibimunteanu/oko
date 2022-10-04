#include "renderer/vulkan/vulkan_backend.h"

#include "renderer/vulkan/vulkan_types.h"
#include "renderer/vulkan/vulkan_device.h"
#include "renderer/vulkan/vulkan_platform.h"
#include "renderer/vulkan/vulkan_swapchain.h"
#include "renderer/vulkan/vulkan_renderpass.h"

#include "core/log.h"

#include "containers/string.h"
#include "containers/darray.h"

// static Vulkan context
static vulkan_context context;

// PRIVATE
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {
    //
    switch (message_severity) {
        default:
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            OKO_ERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            OKO_WARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            OKO_INFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            OKO_TRACE(callback_data->pMessage);
            break;
    }
    return VK_FALSE;
}

i32 find_memory_index(
    u32 type_filter,
    u32 property_flags) {
    //
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        // Check each memory type to see if its bit is set to 1
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }

    OKO_WARN("Unable to find suitable memory type!");

    return -1;
}

// PUBLIC
b8 vulkan_renderer_backend_initialize(
    struct renderer_backend* backend,
    const char* application_name,
    struct platform_state* platform_state) {
    //
    // Function pointers
    context.find_memory_index = find_memory_index;

    // TODO: custom allocator
    context.allocator = 0;

    // Setup Vulkan instance
    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pEngineName = "Oko";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pApplicationName = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

    VkInstanceCreateInfo create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    create_info.pApplicationInfo = &app_info;

    // Obtain a list of required extensions
    const char** required_extensions = darray_create(const char*);
    darray_push(required_extensions, &VK_KHR_SURFACE_EXTENSION_NAME);    // Generic surface extension
    platform_get_vulkan_required_extension_names(&required_extensions);  // Platform-specific extension(s)
#if defined(_DEBUG)
    darray_push(required_extensions, &VK_EXT_DEBUG_UTILS_EXTENSION_NAME);  // Debug utils

    OKO_DEBUG("Required extensions:");
    u32 length = darray_length(required_extensions);
    for (u32 i = 0; i < length; i++) {
        OKO_DEBUG(required_extensions[i]);
    }
#endif

    create_info.enabledExtensionCount = darray_length(required_extensions);
    create_info.ppEnabledExtensionNames = required_extensions;

    // Validation layers
    const char** required_validation_layer_names = 0;
    u32 required_validation_layer_count = 0;

// If validation should be done, get a list of the required validation layer names
// and make sure they exist. Validation layers should only be enabled on non-release builds.
#if defined(_DEBUG)
    OKO_INFO("Validation layers enabled. Enumerating...");

    // The list of validation layers required.
    required_validation_layer_names = darray_create(const char*);
    darray_push(required_validation_layer_names, &"VK_LAYER_KHRONOS_validation");
    required_validation_layer_count = darray_length(required_validation_layer_names);

    // Obtain a list of available validation layers
    u32 available_layer_count = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, 0));
    VkLayerProperties* available_layers = darray_reserve(VkLayerProperties, available_layer_count);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&available_layer_count, available_layers));

    // Verify all required layers are available
    for (u32 i = 0; i < required_validation_layer_count; i++) {
        OKO_INFO("Searching for layer: %s...", required_validation_layer_names[i]);
        b8 found = false;
        for (u32 j = 0; j < available_layer_count; j++) {
            if (strings_equal(required_validation_layer_names[i], available_layers[j].layerName)) {
                found = true;
                OKO_INFO("Found.");
                break;
            }
        }

        if (!found) {
            OKO_FATAL("Required validation layer is missing: %s", required_validation_layer_names[i]);
            return false;
        }
    }
    OKO_INFO("All required validation layers are present.");
#endif

    create_info.enabledLayerCount = required_validation_layer_count;
    create_info.ppEnabledLayerNames = required_validation_layer_names;

    VK_CHECK(vkCreateInstance(&create_info, context.allocator, &context.instance));
    OKO_INFO("Vulkan instance created.")

    // Debugger
#if defined(_DEBUG)
    OKO_DEBUG("Creating Vulkan debugger...");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;  // |
                                                                         //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                                         //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = vulkan_debug_callback;

    // NOTE: since this is an extension, we have to load a function pointer to it
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkCreateDebugUtilsMessengerEXT");
    OKO_ASSERT_MSG(func, "Failed to get PFN_vkCreateDebugUtilsMessengerEXT!");
    VK_CHECK(func(context.instance, &debug_create_info, context.allocator, &context.debug_messenger));
    OKO_DEBUG("Vulkan debugger created.");
#endif

    // Surface creation
    OKO_DEBUG("Creating Vulkan surface...");
    if (!platform_create_vulkan_surface(platform_state, &context)) {
        OKO_ERROR("Failed to create platform surface!");
        return false;
    }
    OKO_DEBUG("Vulkan surface created.");

    // Device creation
    if (!vulkan_device_create(&context)) {
        OKO_ERROR("Failed to create vulkan device!");
        return false;
    }

    // Swapchain creation
    vulkan_swapchain_create(
        &context,
        context.framebuffer_width,
        context.framebuffer_height,
        &context.swapchain);

    // Renderpass creation
    vulkan_renderpass_create(
        &context,
        &context.main_renderpass,
        0, 0, context.framebuffer_width, context.framebuffer_height,
        0.0f, 0.0f, 0.2f, 1.0f,
        1.0f,
        0);

    OKO_INFO("Vulkan renderer initialized successfully!")
    return true;
}

void vulkan_renderer_backend_shutdown(
    struct renderer_backend* backend) {
    //
    OKO_DEBUG("Destroying vulkan renderpass...");
    vulkan_renderpass_destroy(&context, &context.main_renderpass);

    OKO_DEBUG("Destroying vulkan swapchain...");
    vulkan_swapchain_destroy(&context, &context.swapchain);

    OKO_DEBUG("Destroying vulkan device...");
    vulkan_device_destroy(&context);

    OKO_DEBUG("Destroying vulkan surface...");
    if (context.surface) {
        vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);
        context.surface = 0;
    }

#if defined(_DEBUG)
    OKO_DEBUG("Destroying Vulkan debugger...");
    if (context.debug_messenger) {
        PFN_vkDestroyDebugUtilsMessengerEXT func =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context.instance, "vkDestroyDebugUtilsMessengerEXT");
        func(context.instance, context.debug_messenger, context.allocator);
    }
#endif

    OKO_DEBUG("Destroying Vulkan instance...");
    vkDestroyInstance(context.instance, context.allocator);
}

void vulkan_renderer_backend_resized(
    struct renderer_backend* backend,
    u16 width,
    u16 height) {
    //
}

b8 vulkan_renderer_backend_begin_frame(
    struct renderer_backend* backend,
    f32 delta_time) {
    //
    return true;
}

b8 vulkan_renderer_backend_end_frame(
    struct renderer_backend* backend,
    f32 delta_time) {
    //
    return true;
}
