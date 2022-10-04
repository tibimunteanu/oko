#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"

#include "core/log.h"
#include "core/memory.h"

// Backend render context
static renderer_backend* backend = 0;

b8 renderer_initialize(
    const char* application_name,
    struct platform_state* platform_state) {
    //
    backend = memory_allocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);

    // TODO: make this configurable
    renderer_backend_create(RENDERER_BACKEND_VULKAN, platform_state, backend);

    if (!backend->initialize(backend, application_name, platform_state)) {
        OKO_FATAL("Renderer backend failed to initialize. Shutting down.")
        return false;
    }

    return true;
}

void renderer_shutdown() {
    backend->shutdown(backend);
    memory_free(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

b8 renderer_begin_frame(f32 delta_time) {
    return backend->begin_frame(backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
    b8 result = backend->end_frame(backend, delta_time);
    backend->frame_number++;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    // If the begin frame returned successfully, mid-frame operations may continue.
    if (renderer_begin_frame(packet->delta_time)) {
        // End the frame. If this fails, it is likely unrecoverable.
        b8 result = renderer_end_frame(packet->delta_time);

        if (!result) {
            OKO_ERROR("renderer_end_frame failed. Application shutting down...");
            return false;
        }
    }
    return true;
}