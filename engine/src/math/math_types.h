#pragma once

#include "defines.h"

typedef union vec2_u {
    f32 elements[2];
    struct {
        union {
            f32 x, r, s, u;
        };
        union {
            f32 y, g, t, v;
        };
    };
} vec2;

typedef union vec3_u {
    f32 elements[3];
    struct {
        union {
            f32 x, r, s, u;
        };
        union {
            f32 y, g, t, v;
        };
        union {
            f32 z, b, p, w;
        };
    };
} vec3;

typedef union vec4_u {
    f32 elements[4];
    struct {
        union {
            f32 x, r, s;
        };
        union {
            f32 y, g, t;
        };
        union {
            f32 z, b, p;
        };
        union {
            f32 w, a, q;
        };
    };
} vec4;

typedef vec4 quat;

typedef union mat4_u {
    f32 data[16];
} mat4;

typedef struct vertex_3d {
    vec3 position;
    vec2 texcoord;
} vertex_3d;