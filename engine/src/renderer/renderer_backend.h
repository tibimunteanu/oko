#pragma once

#include "renderer/renderer_types.h"

struct platform_state;

b8 renderer_backend_create(renderer_backend_types type, struct platform_state* platform_state, renderer_backend* out_renderer_backend);
void renderer_backend_destroy(renderer_backend* renderer_backend);