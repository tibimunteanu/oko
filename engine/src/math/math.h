#pragma once

#include "defines.h"
#include "math_types.h"

#define OKO_PI                  3.14159265358979323846f
#define OKO_PI_2                2.0f * OKO_PI
#define OKO_HALF_PI             0.5f * OKO_PI
#define OKO_QUARTER_PI          0.25f * OKO_PI
#define OKO_ONE_OVER_PI         1.0f / OKO_PI
#define OKO_ONE_OVER_2PI        1.0f / OKO_PI_2
#define OKO_SQRT_TWO            1.41421356237309504880f
#define OKO_SQRT_THREE          1.73205080756887729352f
#define OKO_SQRT_ONE_OVER_TWO   0.70710678118654752440f
#define OKO_SQRT_ONE_OVER_THREE 0.57735026918962576450f
#define OKO_DEG2RAD             OKO_PI / 180.0f
#define OKO_RAD2DEG             180.0f / OKO_PI

#define OKO_SEC_TO_MS_MULTIPLIER 1000.0f
#define OKO_MS_TO_SEC_MULTIPLIER 0.001f

#define OKO_INFINITY      1e+30f
#define OKO_FLOAT_EPSILON 1.192092896e-07f

OKO_API f32 oko_sin(f32 x);
OKO_API f32 oko_cos(f32 x);
OKO_API f32 oko_tan(f32 x);
OKO_API f32 oko_acos(f32 x);
OKO_API f32 oko_sqrt(f32 x);
OKO_API f32 oko_abs(f32 x);

OKO_INLINE b8 is_power_of_2(u32 value) {
    return (value != 0) && ((value & (value - 1)) == 0);
}

OKO_API i32 oko_random();
OKO_API i32 oko_random_in_range(i32 min, i32 max);

OKO_API f32 oko_random_f();
OKO_API f32 oko_random_in_range_f(f32 min, f32 max);

// vec2 functions
OKO_INLINE vec2 vec2_create(f32 x, f32 y) {
    vec2 out_vector;
    out_vector.x = x;
    out_vector.y = y;
    return out_vector;
}

OKO_INLINE vec2 vec2_zero() {
    return (vec2) {0.0f, 0.0f};
}

OKO_INLINE vec2 vec2_one() {
    return (vec2) {1.0f, 1.0f};
}

OKO_INLINE vec2 vec2_up() {
    return (vec2) {0.0f, 1.0f};
}

OKO_INLINE vec2 vec2_down() {
    return (vec2) {0.0f, -1.0f};
}

OKO_INLINE vec2 vec2_left() {
    return (vec2) {-1.0f, 0.0f};
}

OKO_INLINE vec2 vec2_right() {
    return (vec2) {1.0f, 0.0f};
}

OKO_INLINE vec2 vec2_add(vec2 a, vec2 b) {
    return (vec2) {a.x + b.x, a.y + b.y};
}

OKO_INLINE vec2 vec2_sub(vec2 a, vec2 b) {
    return (vec2) {a.x - b.x, a.y - b.y};
}

OKO_INLINE vec2 vec2_mul(vec2 a, vec2 b) {
    return (vec2) {a.x * b.x, a.y * b.y};
}

OKO_INLINE vec2 vec2_div(vec2 a, vec2 b) {
    return (vec2) {a.x / b.x, a.y / b.y};
}

OKO_INLINE vec2 vec2_scale(vec2 a, f32 scale) {
    return (vec2) {a.x * scale, a.y * scale};
}

OKO_INLINE f32 vec2_dot(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

OKO_INLINE f32 vec2_length_squared(vec2 a) {
    return a.x * a.x + a.y * a.y;
}

OKO_INLINE f32 vec2_length(vec2 a) {
    return oko_sqrt(vec2_length_squared(a));
}

OKO_INLINE void vec2_normalize(vec2* a) {
    f32 length = vec2_length(*a);
    a->x /= length;
    a->y /= length;
}

OKO_INLINE vec2 vec2_normalized(vec2 a) {
    f32 length = vec2_length(a);
    return (vec2) {a.x / length, a.y / length};
}

OKO_INLINE f32 vec2_distance_squared(vec2 a, vec2 b) {
    return vec2_length_squared(vec2_sub(a, b));
}

OKO_INLINE f32 vec2_distance(vec2 a, vec2 b) {
    return vec2_length(vec2_sub(a, b));
}

OKO_INLINE b8 vec2_compare(vec2 a, vec2 b, f32 tolerance) {
    return (oko_abs(a.x - b.x) < tolerance) && (oko_abs(a.y - b.y) < tolerance);
}

// vec3 functions

OKO_INLINE vec3 vec3_create(f32 x, f32 y, f32 z) {
    vec3 out_vector;
    out_vector.x = x;
    out_vector.y = y;
    out_vector.z = z;
    return out_vector;
}

OKO_INLINE vec3 vec3_zero() {
    return (vec3) {0.0f, 0.0f, 0.0f};
}

OKO_INLINE vec3 vec3_one() {
    return (vec3) {1.0f, 1.0f, 1.0f};
}

OKO_INLINE vec3 vec3_up() {
    return (vec3) {0.0f, 1.0f, 0.0f};
}

OKO_INLINE vec3 vec3_down() {
    return (vec3) {0.0f, -1.0f, 0.0f};
}

OKO_INLINE vec3 vec3_left() {
    return (vec3) {-1.0f, 0.0f, 0.0f};
}

OKO_INLINE vec3 vec3_right() {
    return (vec3) {1.0f, 0.0f, 0.0f};
}

OKO_INLINE vec3 vec3_forward() {
    return (vec3) {0.0f, 0.0f, 1.0f};
}

OKO_INLINE vec3 vec3_back() {
    return (vec3) {0.0f, 0.0f, -1.0f};
}

OKO_INLINE vec3 vec3_add(vec3 a, vec3 b) {
    return (vec3) {a.x + b.x, a.y + b.y, a.z + b.z};
}

OKO_INLINE vec3 vec3_sub(vec3 a, vec3 b) {
    return (vec3) {a.x - b.x, a.y - b.y, a.z - b.z};
}

OKO_INLINE vec3 vec3_mul(vec3 a, vec3 b) {
    return (vec3) {a.x * b.x, a.y * b.y, a.z * b.z};
}

OKO_INLINE vec3 vec3_div(vec3 a, vec3 b) {
    return (vec3) {a.x / b.x, a.y / b.y, a.z / b.z};
}

OKO_INLINE vec3 vec3_scale(f32 a, vec3 b) {
    return (vec3) {a * b.x, a * b.y, a * b.z};
}

OKO_INLINE f32 vec3_dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

OKO_INLINE vec3 vec3_cross(vec3 a, vec3 b) {
    return (vec3) {
        a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

OKO_INLINE f32 vec3_length_squared(vec3 a) {
    return a.x * a.x + a.y * a.y + a.z * a.z;
}

OKO_INLINE f32 vec3_length(vec3 a) {
    return oko_sqrt(vec3_length_squared(a));
}

OKO_INLINE void vec3_normalize(vec3* a) {
    f32 length = vec3_length(*a);
    a->x /= length;
    a->y /= length;
    a->z /= length;
}

OKO_INLINE vec3 vec3_normalized(vec3 a) {
    f32 length = vec3_length(a);
    return (vec3) {a.x / length, a.y / length, a.z / length};
}

OKO_INLINE f32 vec3_distance(vec3 a, vec3 b) {
    return vec3_length(vec3_sub(a, b));
}

OKO_INLINE f32 vec3_distance_squared(vec3 a, vec3 b) {
    return vec3_length_squared(vec3_sub(a, b));
}

OKO_INLINE b8 vec3_compare(vec3 a, vec3 b, f32 tolerance) {
    return (oko_abs(a.x - b.x) < tolerance) &&
           (oko_abs(a.y - b.y) < tolerance) && (oko_abs(a.z - b.z) < tolerance);
}

OKO_INLINE vec3 vec3_lerp(vec3 a, vec3 b, f32 t) {
    return vec3_add(a, vec3_mul(vec3_sub(b, a), (vec3) {t, t, t}));
}

OKO_INLINE vec3 vec3_reflect(vec3 a, vec3 normal) {
    return vec3_sub(
        a,
        vec3_mul(
            normal,
            (vec3) {
                2.0f * vec3_dot(a, normal),
                2.0f * vec3_dot(a, normal),
                2.0f * vec3_dot(a, normal)}
        )
    );
}

OKO_INLINE vec3 vec3_project(vec3 a, vec3 b) {
    return vec3_mul(
        b,
        (vec3) {
            vec3_dot(a, b) / vec3_length_squared(b),
            vec3_dot(a, b) / vec3_length_squared(b),
            vec3_dot(a, b) / vec3_length_squared(b)}
    );
}

OKO_INLINE vec3 vec3_rotate_x(vec3 a, f32 angle) {
    f32 cos_angle = oko_cos(angle);
    f32 sin_angle = oko_sin(angle);
    return (vec3) {
        a.x,
        a.y * cos_angle - a.z * sin_angle,
        a.y * sin_angle + a.z * cos_angle};
}

OKO_INLINE vec3 vec3_rotate_y(vec3 a, f32 angle) {
    f32 cos_angle = oko_cos(angle);
    f32 sin_angle = oko_sin(angle);
    return (vec3) {
        a.x * cos_angle + a.z * sin_angle,
        a.y,
        -a.x * sin_angle + a.z * cos_angle};
}

OKO_INLINE vec3 vec3_rotate_z(vec3 a, f32 angle) {
    f32 cos_angle = oko_cos(angle);
    f32 sin_angle = oko_sin(angle);
    return (vec3) {
        a.x * cos_angle - a.y * sin_angle,
        a.x * sin_angle + a.y * cos_angle,
        a.z};
}

OKO_INLINE vec3 vec3_rotate(vec3 a, vec3 axis, f32 angle) {
    f32 cos_angle = oko_cos(angle);
    f32 sin_angle = oko_sin(angle);
    return vec3_add(
        vec3_add(
            vec3_mul(a, (vec3) {cos_angle, cos_angle, cos_angle}),
            vec3_mul(
                vec3_cross(axis, a), (vec3) {sin_angle, sin_angle, sin_angle}
            )
        ),
        vec3_mul(
            vec3_mul(axis, a),
            (vec3) {1.0f - cos_angle, 1.0f - cos_angle, 1.0f - cos_angle}
        )
    );
}

OKO_INLINE vec3 vec3_from_vec4(vec4 a) {
    return (vec3) {a.x, a.y, a.z};
}

OKO_INLINE vec4 vec3_to_vec4(vec3 a, f32 w) {
    return (vec4) {a.x, a.y, a.z, w};
}

// vec4 functions

OKO_INLINE vec4 vec4_create(f32 x, f32 y, f32 z, f32 w) {
    return (vec4) {x, y, z, w};
}

OKO_INLINE vec4 vec4_zero() {
    return (vec4) {0.0f, 0.0f, 0.0f, 0.0f};
}

OKO_INLINE vec4 vec4_one() {
    return (vec4) {1.0f, 1.0f, 1.0f, 1.0f};
}

OKO_INLINE vec3 vec4_to_vec3(vec4 a) {
    return (vec3) {a.x, a.y, a.z};
}

OKO_INLINE vec4 vec4_from_vec3(vec3 a, f32 w) {
    return (vec4) {a.x, a.y, a.z, w};
}

OKO_INLINE vec4 vec4_add(vec4 a, vec4 b) {
    return (vec4) {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

OKO_INLINE vec4 vec4_sub(vec4 a, vec4 b) {
    return (vec4) {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

OKO_INLINE vec4 vec4_mul(vec4 a, vec4 b) {
    return (vec4) {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

OKO_INLINE vec4 vec4_div(vec4 a, vec4 b) {
    return (vec4) {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

OKO_INLINE vec4 vec4_scale(vec4 a, f32 b) {
    return (vec4) {a.x * b, a.y * b, a.z * b, a.w * b};
}

OKO_INLINE vec4 vec4_dot(vec4 a, vec4 b) {
    return (vec4) {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

OKO_INLINE f32 vec4_length_squared(vec4 a) {
    return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

OKO_INLINE f32 vec4_length(vec4 a) {
    return oko_sqrt(vec4_length_squared(a));
}

OKO_INLINE void vec4_normalize(vec4* a) {
    f32 length = vec4_length(*a);
    a->x /= length;
    a->y /= length;
    a->z /= length;
    a->w /= length;
}

OKO_INLINE vec4 normalized(vec4 a) {
    f32 length = vec4_length(a);
    return (vec4) {a.x / length, a.y / length, a.z / length, a.w / length};
}

OKO_INLINE f32
vec4_dot_f32(f32 x1, f32 y1, f32 z1, f32 w1, f32 x2, f32 y2, f32 z2, f32 w2) {
    return x1 * x2 + y1 * y2 + z1 * z2 + w1 * w2;
}
