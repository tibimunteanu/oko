#pragma once

#include "defines.h"

typedef struct event_context {
    union {
        i64 i64[2];
        u64 u64[2];
        f64 f64[2];

        i32 i32[4];
        u32 u32[4];
        f32 f32[4];

        i16 i16[8];
        u16 u16[8];

        i8 i8[16];
        u8 u8[16];

        char c[16];
    } data;
} event_context;

// Should return true if handled
typedef b8 (*PFN_on_event
)(u16 code, void* sender, void* listener_inst, event_context data);

b8 event_system_initialize(u64* memory_requirement, void* state);
void event_system_shutdown(void* state);

OKO_API b8 event_register(u16 code, void* listener, PFN_on_event on_event);
OKO_API b8 event_unregister(u16 code, void* listener, PFN_on_event on_event);
OKO_API b8 event_fire(u16 code, void* sender, event_context context);

typedef enum event_system_codes {
    EVENT_APPLICATION_QUIT = 0x01,
    EVENT_KEY_PRESSED = 0x02,
    EVENT_KEY_RELEASED = 0x03,
    EVENT_BUTTON_PRESSED = 0x04,
    EVENT_BUTTON_RELEASED = 0x05,
    EVENT_MOUSE_MOVED = 0x06,
    EVENT_MOUSE_WHEEL = 0x07,
    EVENT_RESIZED = 0x08,

    EVENT_CODE_DEBUG0 = 0x10,
    EVENT_CODE_DEBUG1 = 0x11,
    EVENT_CODE_DEBUG2 = 0x12,
    EVENT_CODE_DEBUG3 = 0x13,
    EVENT_CODE_DEBUG4 = 0x14,

    EVENT_MAX_CODE = 0xFF
} event_system_codes;