#include "math.h"
#include "platform/platform.h"

#include <math.h>
#include <stdlib.h>

static b8 rand_seeded = false;

/**
 * Note that these are here in order to prevent having to import the
 * entire <math.h> everywhere.
 */
f32 oko_sin(f32 x) {
    return sinf(x);
}

f32 oko_cos(f32 x) {
    return cosf(x);
}

f32 oko_tan(f32 x) {
    return tanf(x);
}

f32 oko_acos(f32 x) {
    return acosf(x);
}

f32 oko_sqrt(f32 x) {
    return sqrtf(x);
}

f32 oko_abs(f32 x) {
    return fabsf(x);
}

i32 oko_random() {
    if (!rand_seeded) {
        srand((u32)platform_get_absolute_time());
        rand_seeded = true;
    }
    return rand();
}

i32 oko_random_in_range(i32 min, i32 max) {
    if (!rand_seeded) {
        srand((u32)platform_get_absolute_time());
        rand_seeded = true;
    }
    return (rand() % (max - min + 1)) + min;
}

f32 oko_random_f() {
    return (float)oko_random() / (f32)RAND_MAX;
}

f32 oko_random_in_range_f(f32 min, f32 max) {
    return min + ((float)oko_random() / ((f32)RAND_MAX / (max - min)));
}