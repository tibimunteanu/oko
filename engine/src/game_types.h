#pragma once

#include "core/application.h"

// The basic state in a game
typedef struct game {
    application_config app_config;

    b8 (*initialize)(struct game* game_inst);
    b8 (*update)(struct game* game_inst, f32 delta_time);
    b8 (*render)(struct game* game_inst, f32 delta_time);
    void (*on_resize)(struct game* game_inst, u32 width, u32 height);

    // Game-specific game state. Created and managed by the game.
    void* state;

    // Application state.
    void* application_state;
} game;