#include "renderer/renderer_backend.h"

#include "vulkan/vulkan_backend.h"

b8 renderer_backend_create(renderer_backend_types type, struct platform_state* platform_state, renderer_backend* out_renderer_backend) {
    out_renderer_backend->platform_state = platform_state;
    out_renderer_backend->frame_number = 0;

    if (type == RENDERER_BACKEND_VULKAN) {
        out_renderer_backend->initialize = vulkan_renderer_backend_initialize;
        out_renderer_backend->shutdown = vulkan_renderer_backend_shutdown;
        out_renderer_backend->begin_frame = vulkan_renderer_backend_begin_frame;
        out_renderer_backend->end_frame = vulkan_renderer_backend_end_frame;
        out_renderer_backend->resized = vulkan_renderer_backend_resized;
        return true;
    }

    return false;
}

void renderer_backend_destroy(renderer_backend* renderer_backend) {
    renderer_backend->initialize = 0;
    renderer_backend->shutdown = 0;
    renderer_backend->begin_frame = 0;
    renderer_backend->end_frame = 0;
    renderer_backend->resized = 0;
}