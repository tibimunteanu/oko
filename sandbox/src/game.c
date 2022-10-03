#include "game.h"

// Define the function to create a game
b8 create_game(game* out_game) {
    // Set application config
    out_game->app_config.name = "Oko Sandbox";
    out_game->app_config.start_pos_x = 100;
    out_game->app_config.start_pos_y = 100;
    out_game->app_config.start_width = 1280;
    out_game->app_config.start_height = 720;

    // Hook up the game function pointers
    out_game->initialize = game_initialize;
    out_game->update = game_update;
    out_game->render = game_render;
    out_game->on_resize = game_on_resize;

    // Create the game state
    out_game->state = memory_allocate(sizeof(game_state), MEMORY_TAG_GAME);

    return true;
}

b8 game_initialize(game* game_inst) {
    OKO_DEBUG("game_initialize() called!");
    return true;
}
b8 game_update(game* game_inst, f32 delta_time) {
    return true;
}
b8 game_render(game* game_inst, f32 delta_time) {
    return true;
}
void game_on_resize(game* game_inst, u32 width, u32 height) {
}