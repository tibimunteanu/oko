#include "core/application.h"
#include "core/log.h"

#include "platform/platform.h"
#include "core/memory.h"
#include "core/event.h"
#include "core/input.h"

#include "game_types.h"

typedef struct application_state {
    game* game_inst;
    b8 is_running;
    b8 is_suspended;
    platform_state platform;
    i16 width;
    i16 height;
    f64 last_time;
} application_state;

static b8 initialized = false;
static application_state app_state;

// Event handlers
b8 application_on_event(u16 code, void* sender, void* listener_inst, event_context context);
b8 application_on_key(u16 code, void* sender, void* listener_inst, event_context context);

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

    if (!platform_startup(&app_state.platform, game_inst->app_config.name,
                          game_inst->app_config.start_pos_x, game_inst->app_config.start_pos_y,
                          game_inst->app_config.start_width, game_inst->app_config.start_height)) {
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
    OKO_INFO(memory_get_usage_string());

    while (app_state.is_running) {
        if (!platform_pump_messages(&app_state.platform)) {
            app_state.is_running = false;
            break;
        }

        if (!app_state.is_suspended) {
            input_update(0);

            // Update game
            if (!app_state.game_inst->update(app_state.game_inst, (f32)0)) {
                OKO_FATAL("Game update failed, shutting down!");
                app_state.is_running = false;
                break;
            }

            // Render game
            if (!app_state.game_inst->render(app_state.game_inst, (f32)0)) {
                OKO_FATAL("Game render failed, shutting down!");
                app_state.is_running = false;
                break;
            }
        }
    }

    app_state.is_running = false;

    event_unregister(EVENT_APPLICATION_QUIT, 0, application_on_event);
    event_unregister(EVENT_KEY_PRESSED, 0, application_on_key);
    event_unregister(EVENT_KEY_RELEASED, 0, application_on_key);
    event_shutdown();

    input_shutdown();
    log_shutdown();

    platform_shutdown(&app_state.platform);

    return true;
}

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