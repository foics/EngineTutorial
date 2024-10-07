#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glad/glad.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "engine/global.h"
#include "engine/config.h"
#include "engine/input.h"
#include "engine/time.h"
#include "engine/physics.h"
#include "engine/entity.h"
#include "engine/render.h"
#include "engine/animation.h"
#include "engine/audio.h"

static Mix_Music *MUSIC_STAGE_1;
static Mix_Chunk *SOUND_JUMP;

static const f32 SPEED_ENEMY_LARGE = 200;
static const f32 SPEED_ENEMY_SMALL = 4000;
static const f32 HEALTH_ENEMY_LARGE = 7;
static const f32 HEALTH_ENEMY_SMALL = 3;

typedef enum collision_layer {
    COLLISION_LAYER_PLAYER = 1,
    COLLISION_LAYER_ENEMY = 1 << 1,
    COLLISION_LAYER_TERRAIN = 1 << 2,
    COLLISION_LAYER_ENEMY_PASSTHROUGH = 1 << 3,
} Collision_Layer;

static bool should_quit = false;

vec4 player_color = {1, 0, 0.5, 1};
bool player_is_grounded = false;

static void input_handle(Body *body_player) {
    if (global.input.escape) {
        should_quit = true;
    }

    f32 velx = 0;
    f32 vely = body_player->velocity[1];

    if (global.input.right) {
        velx += 300;
    }

    if (global.input.left) {
        velx -= 300;
    }

    if (global.input.up && player_is_grounded) {
        player_is_grounded = false;
        vely = 1400;
        audio_sound_play(SOUND_JUMP);
    }

    body_player->velocity[0] = velx;
    body_player->velocity[1] = vely;
}

void player_on_hit(Body *self, Body *other, Hit hit) {
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        player_color[0] = 0.5;
        player_color[2] = 1;
    }
}

void player_on_hit_static(Body *self, Static_Body *other, Hit hit) {
    if (hit.normal[1] > 0) {
        player_is_grounded = true;
    }
}

void enemy_small_on_hit_static(Body *self, Static_Body *other, Hit hit) {
    if (hit.normal[0] > 0) {
        self->velocity[0] = SPEED_ENEMY_SMALL;
    }

    if (hit.normal[0] < 0) {
        self->velocity[0] = -SPEED_ENEMY_SMALL;
    }
}

void enemy_large_on_hit_static(Body *self, Static_Body *other, Hit hit) {
    if (hit.normal[0] > 0) {
        self->velocity[0] = SPEED_ENEMY_LARGE;
    }

    if (hit.normal[0] < 0) {
        self->velocity[0] = -SPEED_ENEMY_LARGE;
    }
}

void fire_on_hit(Body *self, Body *other, Hit hit) {
    if (other->collision_layer == COLLISION_LAYER_ENEMY) {
        for (size_t i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);

            if (entity->body_id == hit.other_id) {
                Body *body = physics_body_get(entity->body_id);
                body->is_active = false;
                entity->is_active = false;
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    time_init(60);
    SDL_Window *window = render_init();
    config_init();
    physics_init();
    entity_init();
    animation_init();
    audio_init();

    audio_sound_load(&SOUND_JUMP, "assets/jump.wav");
    audio_music_load(&MUSIC_STAGE_1, "assets/breezys_mega_quest_2_stage_1.mp3");
    audio_music_play(MUSIC_STAGE_1);

    SDL_ShowCursor(false);

    u8 enemy_mask = COLLISION_LAYER_PLAYER | COLLISION_LAYER_TERRAIN;
    u8 player_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_TERRAIN;
    u8 fire_mask = COLLISION_LAYER_ENEMY | COLLISION_LAYER_PLAYER;

    i32 window_width, window_height;
    SDL_GetWindowSize(window, &window_width, &window_height);
    f32 width = window_width / render_get_scale();
    f32 height = window_height / render_get_scale();

    size_t player_id = entity_create((vec2){100, 200}, (vec2){24, 24}, (vec2){0, 0}, (vec2){0, 0}, COLLISION_LAYER_PLAYER, player_mask, false, (size_t)-1, player_on_hit, player_on_hit_static);

    // init level
    physics_static_body_create((vec2){width * 0.5, height - 16}, (vec2){width, 32}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){width * 0.25 - 16, 16}, (vec2){width * 0.5 - 32, 48}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){width * 0.75 + 16, 16}, (vec2){width * 0.5 - 32, 48}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){16, height * 0.5 - 3 * 32}, (vec2){32, height}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){width - 16, height * 0.5 - 3 * 32}, (vec2){32, height}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){32 + 64, height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){width - 32 - 64, height - 32 * 3 - 16}, (vec2){128, 32}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){width * 0.5, height - 32 * 3 - 16}, (vec2){192, 32}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){width * 0.5, 32 * 3 + 24}, (vec2){448, 32}, COLLISION_LAYER_TERRAIN);
    physics_static_body_create((vec2){16, height - 64}, (vec2){32, 64}, COLLISION_LAYER_ENEMY_PASSTHROUGH);
    physics_static_body_create((vec2){width - 16, height - 64}, (vec2){32, 64}, COLLISION_LAYER_ENEMY_PASSTHROUGH);

    Sprite_Sheet sprite_sheet_player;
    Sprite_Sheet sprite_sheet_map;
    render_sprite_sheet_init(&sprite_sheet_player, "assets/player.png", 24, 24);
    render_sprite_sheet_init(&sprite_sheet_map, "assets/map.png", 640, 360);

    size_t adef_player_walk_id = animation_definition_create(
            &sprite_sheet_player,
            (f32[]){0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1},
            (u8[]){0, 0, 0, 0, 0, 0, 0},
            (u8[]){1, 2, 3, 4, 5, 6, 7},
            7);
    size_t adef_player_idle_id = animation_definition_create(&sprite_sheet_player, (f32[]){0}, (u8[]){0}, (u8[]){0}, 1);
    size_t anim_player_walk_id = animation_create(adef_player_walk_id, true);
    size_t anim_player_idle_id = animation_create(adef_player_idle_id, false);

    Entity *player = entity_get(player_id);
    player->animation_id = anim_player_idle_id;

    f32 spawn_timer = 0;
    u32 texture_slots[8] = {0};

    while (!should_quit) {
        time_update();

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                should_quit = true;
                break;
            default:
                break;
            }
        }

        Entity *player = entity_get(player_id);
        Body *body_player = physics_body_get(player->body_id);

        if (body_player->velocity[0] == 0) {
            player->animation_id = anim_player_idle_id;
        } else {
            player->animation_id = anim_player_walk_id;
        }

        input_update();
        input_handle(body_player);
        physics_update();
        animation_update(global.time.delta);

        // Spawn enemies.
        spawn_timer -= global.time.delta;
        if (spawn_timer <= 0) {
            spawn_timer = (f32)((rand() % 200) + 200) / 100.f;
            spawn_timer *= 0.2;

            bool is_flipped = rand() % 100 >= 50;
            bool is_small_entity = rand() % 100 > 18;

            f32 spawn_x = is_flipped ? 540 : 100;
        }

        render_begin();

        render_sprite_sheet_frame(&sprite_sheet_map, 0, 0, (vec2){width / 2.0, height / 2.0}, false, (vec4){1, 1, 1, 0.2}, texture_slots);

        for (size_t i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            Body *body = physics_body_get(entity->body_id);

            if (body->is_active) {
                render_aabb((f32*)body, (vec4){1, 0.5, 0, 1});
            } else {
                render_aabb((f32*)body, (vec4){0, 0.5, 1, 1});
            }
        }

        for (size_t i = 0; i < physics_static_body_count(); i++) {
            render_aabb((f32*)physics_static_body_get(i), (vec4){1, 1, 1, 1});
        }

        // Render animated entities...
        for (size_t i = 0; i < entity_count(); i++) {
            Entity *entity = entity_get(i);
            if (!entity->is_active || entity->animation_id == (size_t)-1) {
                continue;
            }

            Body *body = physics_body_get(entity->body_id);
            Animation *anim = animation_get(entity->animation_id);
            Animation_Definition *adef = anim->definition;
            Animation_Frame *aframe = &adef->frames[anim->current_frame_index];

            if (body->velocity[0] < 0) {
                anim->is_flipped = true;
            } else if (body->velocity[0] > 0) {
                anim->is_flipped = false;
            }

            vec2 pos;

            vec2_add(pos, body->aabb.position, entity->sprite_offset);
            animation_render(anim, pos, (vec4){1, 1, 1, 1}, texture_slots);
        }

        render_end(window, texture_slots);

        player_color[0] = 1;
        player_color[2] = 0.5;

        time_update_late();
    }

    return 0;
}
