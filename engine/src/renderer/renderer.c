#include "renderer/renderer.h"
#include "renderer/renderer_backend.h"

#include "core/log.h"
#include "core/memory.h"

#include "math/math.h"

#include "resources/resource_types.h"
#include "systems/texture_system.h"

// TODO: temporary
#include "containers/string.h"
#include "core/event.h"
// TODO: end temporary

typedef struct renderer_system_state {
    renderer_backend backend;
    mat4 projection;
    mat4 view;
    f32 near_clip;
    f32 far_clip;

    // TODO: temporary
    texture* test_diffuse;
    // TODO: end temporary
} renderer_system_state;

static renderer_system_state* state_ptr;

// TODO: temporary
b8 event_on_debug_event(
    u16 code, void* sender, void* listener_inst, event_context data
) {
    const char* names[3] = {"cobblestone", "paving", "paving2"};

    static i8 choice = 2;

    // save off the old name
    const char* old_name = names[choice];

    choice++;
    choice %= 3;

    // acquire the new texture
    state_ptr->test_diffuse = texture_system_acquire(names[choice], true);

    // release the old texture
    texture_system_release(old_name);

    return true;
}
// TODO: end temporary

b8 renderer_system_initialize(
    u64* memory_requirement, void* state, const char* application_name
) {
    *memory_requirement = sizeof(renderer_system_state);
    if (state == 0) {
        return true;
    }
    state_ptr = state;

    // TODO: temporary
    event_register(EVENT_CODE_DEBUG0, state_ptr, event_on_debug_event);
    // TODO: end temporary

    // TODO: make this configurable
    renderer_backend_create(RENDERER_BACKEND_VULKAN, &state_ptr->backend);
    state_ptr->backend.frame_number = 0;

    if (!state_ptr->backend.initialize(&state_ptr->backend, application_name)) {
        OKO_FATAL(
            "Renderer state_ptr->backend failed to initialize. Shutting down."
        )
        return false;
    }

    state_ptr->near_clip = 0.1f;
    state_ptr->far_clip = 1000.0f;
    state_ptr->projection = mat4_perspective(
        deg_to_rad(45.0f),
        1280 / 720.0f,
        state_ptr->near_clip,
        state_ptr->far_clip
    );

    state_ptr->view = mat4_translation((vec3) {0.0f, 0.0f, 30.0f});
    state_ptr->view = mat4_inverse(state_ptr->view);

    return true;
}

void renderer_system_shutdown(void* state) {
    if (state_ptr) {
        // TODO: temporary
        event_unregister(EVENT_CODE_DEBUG0, state_ptr, event_on_debug_event);
        // TODO: end temporary

        state_ptr->backend.shutdown(&state_ptr->backend);
    }
    state_ptr = 0;
}

void renderer_on_resized(u16 width, u16 height) {
    if (state_ptr) {
        state_ptr->projection = mat4_perspective(
            deg_to_rad(45.0f),
            width / (f32)height,
            state_ptr->near_clip,
            state_ptr->far_clip
        );
        state_ptr->backend.resized(&state_ptr->backend, width, height);
    } else {
        OKO_WARN(
            "Renderer state_ptr->backend does not exist to accept resize: %i %i",
            width,
            height
        );
    }
}

b8 renderer_begin_frame(f32 delta_time) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time) {
    if (!state_ptr) {
        return false;
    }
    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
    state_ptr->backend.frame_number++;
    return result;
}

b8 renderer_draw_frame(render_packet* packet) {
    // If the begin frame returned successfully, mid-frame operations may
    // continue.
    if (renderer_begin_frame(packet->delta_time)) {
        state_ptr->backend.update_global_state(
            state_ptr->projection, state_ptr->view, vec3_zero(), vec4_one(), 0
        );

        // mat4 model = mat4_translation((vec3) {0.0f, 0.0f, 0.0f});
        static f32 angle = 0.0f;
        angle += 2.0f * packet->delta_time;
        // quat rotation = quat_from_axis_angle(vec3_forward(), angle, false);
        // mat4 model = quat_to_rotation_matrix(rotation, vec3_zero());
        mat4 model = mat4_translation((vec3) {0.0f, 0.0f, 0.0f});
        geometry_render_data data = {};
        data.object_id = 0;  // TODO: actual object_id
        data.model = model;

        // TODO: temporary
        // grab the default if does not exist
        if (!state_ptr->test_diffuse) {
            state_ptr->test_diffuse = texture_system_get_default_texture();
        }

        data.textures[0] = state_ptr->test_diffuse;
        state_ptr->backend.update_object(data);

        // End the frame. If this fails, it is likely unrecoverable.
        b8 result = renderer_end_frame(packet->delta_time);

        if (!result) {
            OKO_ERROR("renderer_end_frame failed. Application shutting down..."
            );
            return false;
        }
    }
    return true;
}

void renderer_set_view(mat4 view) {
    state_ptr->view = view;
}

void renderer_create_texture(
    const char* name,
    i32 width,
    i32 height,
    i32 channel_count,
    const u8* pixels,
    b8 has_transparency,
    struct texture* out_texture
) {
    state_ptr->backend.create_texture(
        name,
        width,
        height,
        channel_count,
        pixels,
        has_transparency,
        out_texture
    );
}

void renderer_destroy_texture(struct texture* texture) {
    state_ptr->backend.destroy_texture(texture);
}