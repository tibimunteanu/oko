#pragma once

#include "defines.h"

#include "math/math_types.h"

typedef struct texture {
    u32 id;
    u32 width;
    u32 height;
    u32 channel_count;
    b8 has_transparency;
    u32 generation;
    void* internal_data;
} texture;