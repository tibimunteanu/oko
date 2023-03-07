#include "core/application.h"

#include "core/log.h"
#include "core/memory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"

#include "memory/linear_allocator.h"

#include "renderer/renderer.h"

#include "platform/platform.h"

#include "game_types.h"

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
    linear_allocator systems_allocator;

    u64 event_system_memory_requirement;
    void* event_system_state;

    u64 memory_system_memory_requirement;
    void* memory_system_state;

    u64 log_system_memory_requirement;
    void* log_system_state;

    u64 input_system_memory_requirement;
    void* input_system_state;

    u64 platform_system_memory_requirement;
    void* platform_system_state;

    u64 renderer_system_memory_requirement;
    void* renderer_system_state;
} application_state;

static application_state* app_state;

void application_get_framebuffer_size(u32* width, u32* height) {
    *width = app_state->width;
    *height = app_state->height;
}

// Event handlers
b8 application_on_event(
    u16 code, void* sender, void* listener_inst, event_context context
) {
    switch (code) {
    case EVENT_APPLICATION_QUIT: {
        OKO_INFO("EVENT_APPLICATION_QUIT received, shutting down");
        app_state->is_running = false;
        return true;
    }
    }
    return false;
}

b8 application_on_key(
    u16 code, void* sender, void* listener_inst, event_context context
) {
    if (code == EVENT_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE) {
            // NOTE: Technically firing an event to itself, but there may be
            // other listeners
            event_context data = {};
            event_fire(EVENT_APPLICATION_QUIT, 0, data);

            // Block anything else from processing this
            return true;
        } else {
            OKO_DEBUG("'%c' key pressed!", key_code);
        }
    } else if (code == EVENT_KEY_RELEASED) {
        u16 key_code = context.data.u16[0];
        OKO_DEBUG("'%c' key released!", key_code);
    }

    return false;
}

b8 application_on_resized(
    u16 code, void* sender, void* listener_inst, event_context context
) {
    if (code == EVENT_RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // Check if different. If so, trigger a resize event
        if (width != app_state->width || height != app_state->height) {
            app_state->width = width;
            app_state->height = height;

            OKO_DEBUG("Window resize: %i %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                OKO_INFO("Window minimized. Suspending application.");
                app_state->is_suspended = true;
                return true;
            } else {
                if (app_state->is_suspended) {
                    OKO_INFO("Window restored. Resuming application.");
                    app_state->is_suspended = false;
                }
                app_state->game_inst->on_resize(
                    app_state->game_inst, width, height
                );
                renderer_on_resized(width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this
    return false;
}

// PUBLIC
b8 application_create(game* game_inst) {
    if (game_inst->application_state) {
        OKO_ERROR("application_create called more than once!");
        return false;
    }

    game_inst->application_state =
        memory_allocate(sizeof(application_state), MEMORY_TAG_APPLICATION);

    app_state = game_inst->application_state;
    app_state->game_inst = game_inst;
    app_state->is_running = false;
    app_state->is_suspended = false;

    u64 systems_allocator_size = 64 * 1024 * 1024;  // 64 MB
    linear_allocator_create(
        systems_allocator_size, 0, &app_state->systems_allocator
    );

    // Initialize subsystems

    // event system
    event_system_initialize(&app_state->event_system_memory_requirement, 0);
    app_state->event_system_state = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->event_system_memory_requirement
    );
    if (!event_system_initialize(
            &app_state->event_system_memory_requirement,
            app_state->event_system_state
        )) {
        OKO_ERROR("Events system failed to initialize!");
        return false;
    }

    // memory system
    memory_system_initialize(&app_state->memory_system_memory_requirement, 0);
    app_state->memory_system_state = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->memory_system_memory_requirement
    );
    if (!memory_system_initialize(
            &app_state->memory_system_memory_requirement,
            app_state->memory_system_state
        )) {
        OKO_ERROR("Memory system failed to initialize!");
        return false;
    }

    // log system
    log_system_initialize(&app_state->log_system_memory_requirement, 0);
    app_state->log_system_state = linear_allocator_allocate(
        &app_state->systems_allocator, app_state->log_system_memory_requirement
    );
    if (!log_system_initialize(
            &app_state->log_system_memory_requirement,
            app_state->log_system_state
        )) {
        OKO_ERROR("Logging system failed to initialize!");
        return false;
    }

    // input system
    input_system_initialize(&app_state->input_system_memory_requirement, 0);
    app_state->input_system_state = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->input_system_memory_requirement
    );
    if (!input_system_initialize(
            &app_state->input_system_memory_requirement,
            app_state->input_system_state
        )) {
        OKO_ERROR("Input system failed to initialize!");
        return false;
    }

    event_register(EVENT_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_KEY_RELEASED, 0, application_on_key);
    event_register(EVENT_RESIZED, 0, application_on_resized);

    // platform system
    platform_system_startup(
        &app_state->platform_system_memory_requirement, 0, 0, 0, 0, 0, 0
    );
    app_state->platform_system_state = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->platform_system_memory_requirement
    );
    if (!platform_system_startup(
            &app_state->platform_system_memory_requirement,
            app_state->platform_system_state,
            game_inst->app_config.name,
            game_inst->app_config.start_pos_x,
            game_inst->app_config.start_pos_y,
            game_inst->app_config.start_width,
            game_inst->app_config.start_height
        )) {
        return false;
    }

    // renderer system
    renderer_system_initialize(
        &app_state->renderer_system_memory_requirement, 0, 0
    );
    app_state->renderer_system_state = linear_allocator_allocate(
        &app_state->systems_allocator,
        app_state->renderer_system_memory_requirement
    );
    if (!renderer_system_initialize(
            &app_state->renderer_system_memory_requirement,
            app_state->renderer_system_state,
            game_inst->app_config.name
        )) {
        OKO_FATAL("Failed to initialize the renderer. Aborting application.");
        return false;
    }

    // Initialize the game
    if (!app_state->game_inst->initialize(app_state->game_inst)) {
        OKO_FATAL("Game failed to initialize!");
        return false;
    }

    // call resize once to ensure the proper size has been set
    app_state->game_inst->on_resize(
        app_state->game_inst, app_state->width, app_state->height
    );

    return true;
}

b8 application_run() {
    app_state->is_running = true;

    clock_start(&app_state->clock);
    clock_update(&app_state->clock);
    app_state->last_time = app_state->clock.elapsed;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60.0f;

    OKO_INFO(memory_get_usage_string());

    while (app_state->is_running) {
        if (!platform_pump_messages()) {
            app_state->is_running = false;
            break;
        }

        if (!app_state->is_suspended) {
            // Update clock and get delta time
            clock_update(&app_state->clock);
            f64 current_time = app_state->clock.elapsed;
            f64 delta = (current_time - app_state->last_time);
            f64 frame_start_time = platform_get_absolute_time();

            // Update game
            if (!app_state->game_inst->update(
                    app_state->game_inst, (f32)delta
                )) {
                OKO_FATAL("Game update failed, shutting down!");
                app_state->is_running = false;
                break;
            }

            // Render game
            if (!app_state->game_inst->render(
                    app_state->game_inst, (f32)delta
                )) {
                OKO_FATAL("Game render failed, shutting down!");
                app_state->is_running = false;
                break;
            }

            // TODO: this is temporary
            render_packet packet;
            packet.delta_time = delta;
            renderer_draw_frame(&packet);

            // Figure out how long the frame took
            f64 frame_end_time = platform_get_absolute_time();
            f64 frame_elapsed_time = frame_end_time - frame_start_time;
            running_time += frame_elapsed_time;
            f64 remaining_seconds = target_frame_seconds - frame_elapsed_time;

            if (remaining_seconds > 0) {
                u64 remaining_ms = remaining_seconds * 1000;

                // If there is time left, give it back to the OS.
                b8 limit_frames = false;
                if (remaining_ms > 0 && limit_frames) {
                    platform_sleep(remaining_ms - 1);
                }

                frame_count++;
            }

            // NOTE: Input update/state copying should always be handled
            // after any input should be recorded; I.E. before this line.
            // As a safety, input is the last thing to be updated before
            // this frame ends.
            input_update(delta);

            app_state->last_time = current_time;
        }
    }

    app_state->is_running = false;

    event_unregister(EVENT_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_KEY_RELEASED, 0, application_on_key);

    input_system_shutdown(app_state->input_system_state);
    renderer_system_shutdown(app_state->renderer_system_state);
    platform_system_shutdown(app_state->platform_system_state);
    log_system_shutdown(app_state->log_system_state);
    memory_system_shutdown(app_state->memory_system_state);
    event_system_shutdown(app_state->event_system_state);

    return true;
}
