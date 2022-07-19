#pragma once

#include "core/application.h"
#include "core/log.h"
#include "core/memory.h"
#include "game_types.h"

// Externally-defined function to create a game.
extern b8 create_game(game* out_game);

// The main entry point of the application
int main(void) {
    memory_initialize();

    // Request the game instance from the application.
    game game_inst;
    if (!create_game(&game_inst)) {
        OKO_FATAL("Could not create game!");
        return -1;
    }

    // Ensure the function pointers exist.
    if (!game_inst.render || !game_inst.update || !game_inst.initialize || !game_inst.on_resize) {
        OKO_FATAL("The game's function pointers must be assigned!");
        return -2;
    }

    // Initialize tha application
    if (!application_create(&game_inst)) {
        OKO_INFO("Application failed to create!");
        return 1;
    }

    // Begin the game loop
    if (!application_run()) {
        OKO_INFO("Application did not shutdown gracefully!");
        return 2;
    }

    memory_shutdown();

    return 0;
}