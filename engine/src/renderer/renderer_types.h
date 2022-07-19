#pragma once

#include "defines.h"

typedef enum renderer_backend_types {
    RENDERER_BACKEND_VULKAN,
    RENDERER_BACKEND_OPENGL,
    RENDERER_BACKEND_DIRECTX,
} renderer_backend_types;

typedef struct renderer_backend {
    struct platform_state* platform_state;
    u64 frame_number;

    b8 (*initialize)(struct renderer_backend* backend, const char* application_name, struct platform_state* platform_state);
    void (*shutdown)(struct renderer_backend* backend);
    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);
    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);
} renderer_backend;

typedef struct render_packet {
    f32 delta_time;
} render_packet;