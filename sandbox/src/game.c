#include "game.h"

// HACK: this should not be exposed outside the engine
#include "renderer/renderer.h"

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

    // TODO: This should be set by the application
    out_game->application_state = 0;

    return true;
}

void recalculate_view_matrix(game_state* state) {
    if (!state->camera_view_dirty) {
        return;
    }

    mat4 rotation = mat4_euler_xyz(
        state->camera_euler.x, state->camera_euler.y, state->camera_euler.z
    );
    mat4 translation = mat4_translation(state->camera_position);
    state->view = mat4_inverse(mat4_mul(rotation, translation));

    state->camera_view_dirty = false;
}

void camera_pitch(game_state* state, f32 angle) {
    state->camera_euler.x += angle;

    // clamp to avoid Gimbal lock
    f32 limit = deg_to_rad(89.0f);
    state->camera_euler.x = OKO_CLAMP(state->camera_euler.x, -limit, limit);

    state->camera_view_dirty = true;
}

void camera_yaw(game_state* state, f32 angle) {
    state->camera_euler.y += angle;
    state->camera_view_dirty = true;
}

void camera_roll(game_state* state, f32 angle) {
    state->camera_euler.z += angle;
    state->camera_view_dirty = true;
}

b8 game_initialize(game* game_inst) {
    OKO_DEBUG("game_initialize() called!");

    game_state* state = (game_state*)game_inst->state;

    state->camera_position = (vec3) {0.0f, 0.0f, 30.0f};
    state->camera_euler = vec3_zero();
    state->camera_view_dirty = true;

    return true;
}

b8 game_update(game* game_inst, f32 delta_time) {
    game_state* state = (game_state*)game_inst->state;

    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = memory_get_alloc_count();

    if (input_is_key_up('M') && input_was_key_down('M')) {
        OKO_DEBUG(
            "Allocations: %llu (%llu this frame)",
            alloc_count,
            alloc_count - prev_alloc_count
        );
    }

    // HACK: temp hack to move camera around
    f32 move_speed = 50.0f;
    vec3 velocity = vec3_zero();

    if (input_is_key_down('A') || input_is_key_down(KEY_LEFT)) {
        camera_yaw(state, 1.0f * delta_time);
    }
    if (input_is_key_down('D') || input_is_key_down(KEY_RIGHT)) {
        camera_yaw(state, -1.0f * delta_time);
    }
    if (input_is_key_down(KEY_UP)) {
        camera_pitch(state, 1.0f * delta_time);
    }
    if (input_is_key_down(KEY_DOWN)) {
        camera_pitch(state, -1.0f * delta_time);
    }
    if (input_is_key_down('W')) {
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }
    if (input_is_key_down('S')) {
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }
    if (input_is_key_down('Q')) {
        vec3 forward = mat4_left(state->view);
        velocity = vec3_add(velocity, forward);
    }
    if (input_is_key_down('E')) {
        vec3 backward = mat4_right(state->view);
        velocity = vec3_add(velocity, backward);
    }
    if (input_is_key_down(KEY_SPACE)) {
        velocity.y += 1.0f;
    }
    if (input_is_key_down('X')) {
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_zero();
    if (!vec3_compare(z, velocity, 0.0002f)) {
        vec3_normalize(&velocity);
        state->camera_position.x += velocity.x * move_speed * delta_time;
        state->camera_position.y += velocity.y * move_speed * delta_time;
        state->camera_position.z += velocity.z * move_speed * delta_time;
        state->camera_view_dirty = true;
    }

    recalculate_view_matrix(state);

    // HACK: this should not be exposed outside the engine
    renderer_set_view(state->view);

    return true;
}

b8 game_render(game* game_inst, f32 delta_time) {
    game_state* state = (game_state*)game_inst->state;
    return true;
}

void game_on_resize(game* game_inst, u32 width, u32 height) {
}