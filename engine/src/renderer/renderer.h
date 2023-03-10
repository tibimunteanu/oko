#pragma once

#include "renderer/renderer_types.h"

b8 renderer_system_initialize(
    u64* memory_requirement, void* state, const char* application_name
);

void renderer_system_shutdown(void* state);

void renderer_on_resized(u16 width, u16 height);

b8 renderer_draw_frame(render_packet* packet);

// HACK: this should not be exposed outside the engine
OKO_API void renderer_set_view(mat4 view);