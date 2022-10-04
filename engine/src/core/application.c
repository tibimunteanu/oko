#include "core/application.h"

#include "core/log.h"
#include "core/memory.h"
#include "core/event.h"
#include "core/input.h"
#include "core/clock.h"

#include "renderer/renderer.h"

#include "platform/platform.h"

#include "game_types.h"

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    clock clock;
    f64 last_time;
} application_state;

static b8 initialized = false;
static application_state app_state;

void application_get_framebuffer_size(u32* width, u32* height) {
    *width = app_state.width;
    *height = app_state.height;
}

// Event handlers
b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context) {
    switch (code) {
        case EVENT_APPLICATION_QUIT: {
            OKO_INFO("EVENT_APPLICATION_QUIT received, shutting down");
            app_state.is_running = false;
            return true;
        }
    }
    return false;
}

b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context) {
    if (code == EVENT_KEY_PRESSED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_ESCAPE) {
            // NOTE: Technically firing an event to itself, but there may be other listeners
            event_context data = {};
            event_fire(EVENT_APPLICATION_QUIT, 0, data);

            // Block anything else from processing this
            return true;
        } else if (key_code == KEY_A) {
            OKO_DEBUG("Explicit - A key pressed!");
        } else {
            OKO_DEBUG("'%c' key pressed!", key_code);
        }
    } else if (code == EVENT_KEY_RELEASED) {
        u16 key_code = context.data.u16[0];
        if (key_code == KEY_B) {
            OKO_DEBUG("Explicit - B key released!");
        } else {
            OKO_DEBUG("'%c' key released!", key_code);
        }
    }
    return false;
}

b8 application_on_resized(u16 code, void* sender, void* listener_inst, event_context context) {
    if (code == EVENT_RESIZED) {
        u16 width = context.data.u16[0];
        u16 height = context.data.u16[1];

        // Check if different. If so, trigger a resize event
        if (width != app_state.width || height != app_state.height) {
            app_state.width = width;
            app_state.height = height;

            OKO_DEBUG("Window resize: %i %i", width, height);

            // Handle minimization
            if (width == 0 || height == 0) {
                OKO_INFO("Window minimized. Suspending application.");
                app_state.is_suspended = true;
                return true;
            } else {
                if (app_state.is_suspended) {
                    OKO_INFO("Window restored. Resuming application.");
                    app_state.is_suspended = false;
                }
                app_state.game_inst->on_resize(app_state.game_inst, width, height);
                renderer_on_resized(width, height);
            }
        }
    }

    // Event purposely not handled to allow other listeners to get this
    return false;
}

// PUBLIC
b8 application_create(game* game_inst) {
    if (initialized) {
        OKO_ERROR("application_create called more than once!");
        return false;
    }

    app_state.game_inst = game_inst;

    // Initialize subsystems
    log_initialize();
    input_initialize();

    app_state.is_running = true;
    app_state.is_suspended = false;

    if (!event_initialize()) {
        OKO_ERROR("Event system failed to initialize!");
        return false;
    }

    event_register(EVENT_APPLICATION_QUIT, 0, application_on_event);
    event_register(EVENT_KEY_PRESSED, 0, application_on_key);
    event_register(EVENT_KEY_RELEASED, 0, application_on_key);
    event_register(EVENT_RESIZED, 0, application_on_resized);

    if (!platform_startup(
            &app_state.platform,
            game_inst->app_config.name,
            game_inst->app_config.start_pos_x,
            game_inst->app_config.start_pos_y,
            game_inst->app_config.start_width,
            game_inst->app_config.start_height)) {
        return false;
    }

    // Renderer startup
    if (!renderer_initialize(game_inst->app_config.name, &app_state.platform)) {
        OKO_FATAL("Failed to initialize the renderer. Aborting application.");
        return false;
    }

    // Initialize the game
    if (!app_state.game_inst->initialize(app_state.game_inst)) {
        OKO_FATAL("Game failed to initialize!");
        return false;
    }

    app_state.game_inst->on_resize(app_state.game_inst, app_state.width, app_state.height);

    initialized = true;
    return true;
}

b8 application_run() {
    clock_start(&app_state.clock);
    clock_update(&app_state.clock);
    app_state.last_time = app_state.clock.elapsed;
    f64 running_time = 0;
    u8 frame_count = 0;
    f64 target_frame_seconds = 1.0f / 60.0f;

    OKO_INFO(memory_get_usage_string());

    while (app_state.is_running) {
        if (!platform_pump_messages(&app_state.platform)) {
            app_state.is_running = false;
            break;
        }

        if (!app_state.is_suspended) {
            // Update clock and get delta time
            clock_update(&app_state.clock);
            f64 current_time = app_state.clock.elapsed;
            f64 delta = (current_time - app_state.last_time);
            f64 frame_start_time = platform_get_absolute_time();

            input_update(delta);

            // Update game
            if (!app_state.game_inst->update(app_state.game_inst, (f32)delta)) {
                OKO_FATAL("Game update failed, shutting down!");
                app_state.is_running = false;
                break;
            }

            // Render game
            if (!app_state.game_inst->render(app_state.game_inst, (f32)delta)) {
                OKO_FATAL("Game render failed, shutting down!");
                app_state.is_running = false;
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

            app_state.last_time = current_time;
        }
    }

    app_state.is_running = false;

    event_unregister(EVENT_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_KEY_RELEASED, 0, application_on_key);
    event_shutdown();

    input_shutdown();
    log_shutdown();

    renderer_shutdown();

    platform_shutdown(&app_state.platform);

    return true;
}
