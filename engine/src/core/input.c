#include "core/input.h"

#include "core/event.h"
#include "core/memory.h"
#include "core/log.h"

typedef struct keyboard_state {
    b8 keys[KEY_MAX_KEYS];
} keyboard_state;

typedef struct mouse_state {
    i16 x;
    i16 y;
    b8 buttons[BUTTON_MAX_BUTTONS];
} mouse_state;

typedef struct input_state {
    keyboard_state keyboard_current;
    keyboard_state keyboard_previous;
    mouse_state mouse_current;
    mouse_state mouse_previous;
} input_state;

// Internal input state
static input_state* state_ptr;

b8 input_system_initialize(u64* memory_requirement, void* state) {
    *memory_requirement = sizeof(input_state);
    if (state == 0) {
        return true;
    }
    memory_zero(state, sizeof(input_state));
    state_ptr = state;

    OKO_INFO("Input subsystem initialized!");
    return true;
}

void input_system_shutdown(void* state) {
    state_ptr = 0;
}

void input_update(f64 delta_time) {
    if (!state_ptr) {
        return;
    }

    // Copy current states to previous states
    memory_copy(
        &state_ptr->keyboard_previous,
        &state_ptr->keyboard_current,
        sizeof(keyboard_state)
    );
    memory_copy(
        &state_ptr->mouse_previous,
        &state_ptr->mouse_current,
        sizeof(mouse_state)
    );
}

void input_process_key(keys key, b8 pressed) {
    // Only handle this if the state actually changed
    if (state_ptr->keyboard_current.keys[key] != pressed) {
        // Update internal state
        state_ptr->keyboard_current.keys[key] = pressed;

        // Fire off an event for immediate processing
        event_context context;
        context.data.u16[0] = key;
        event_fire(
            pressed ? EVENT_KEY_PRESSED : EVENT_KEY_RELEASED, 0, context
        );
    }
}

void input_process_button(buttons button, b8 pressed) {
    // Only handle this if the state actually changed
    if (state_ptr->keyboard_current.keys[button] != pressed) {
        // Update internal state
        state_ptr->keyboard_current.keys[button] = pressed;

        // Fire off an event for immediate processing
        event_context context;
        context.data.u16[0] = button;
        event_fire(
            pressed ? EVENT_BUTTON_PRESSED : EVENT_BUTTON_RELEASED, 0, context
        );
    }
}

void input_process_mouse_move(i16 x, i16 y) {
    // Only process if actually different
    if (state_ptr->mouse_current.x != x || state_ptr->mouse_current.y != y) {
        // NOTE: Enable this if debugging
        // OKO_DEBUG("Mouse pos: %i, %i!", x, y);

        // Update internal state
        state_ptr->mouse_current.x = x;
        state_ptr->mouse_current.y = y;

        // Fire the event
        event_context context;
        context.data.u16[0] = x;
        context.data.u16[1] = y;
        event_fire(EVENT_MOUSE_MOVED, 0, context);
    }
}

void input_process_mouse_wheel(i8 z_delta) {
    // NOTE: no internal state to update.

    // Fire the event
    event_context context;
    context.data.u8[0] = z_delta;
    event_fire(EVENT_MOUSE_WHEEL, 0, context);
}

b8 input_is_key_down(keys key) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->keyboard_current.keys[key];
}
b8 input_is_key_up(keys key) {
    if (!state_ptr) {
        return true;
    }
    return !state_ptr->keyboard_current.keys[key];
}
b8 input_was_key_down(keys key) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->keyboard_previous.keys[key];
}
b8 input_was_key_up(keys key) {
    if (!state_ptr) {
        return true;
    }
    return !state_ptr->keyboard_previous.keys[key];
}

b8 input_is_button_down(buttons button) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->mouse_current.buttons[button];
}
b8 input_is_button_up(buttons button) {
    if (!state_ptr) {
        return true;
    }
    return !state_ptr->mouse_current.buttons[button];
}
b8 input_was_button_down(buttons button) {
    if (!state_ptr) {
        return false;
    }
    return state_ptr->mouse_previous.buttons[button];
}
b8 input_was_button_up(buttons button) {
    if (!state_ptr) {
        return false;
    }
    return !state_ptr->mouse_previous.buttons[button];
}

void input_get_mouse_position(i32* x, i32* y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mouse_current.x;
    *y = state_ptr->mouse_current.y;
}
void input_get_previous_mouse_position(i32* x, i32* y) {
    if (!state_ptr) {
        *x = 0;
        *y = 0;
        return;
    }
    *x = state_ptr->mouse_previous.x;
    *y = state_ptr->mouse_previous.y;
}