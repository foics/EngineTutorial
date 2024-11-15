#pragma once

#include <stdbool.h>

#include "render.h"

#define MAX_FRAMES 16

typedef struct animation_frame {
    f32 duration;
    u8 row;
    u8 column;
} Animation_Frame;

typedef struct animation_definition {
    Sprite_Sheet *sprite_sheet;
    Animation_Frame frames[MAX_FRAMES];
    u8 frame_count;
} Animation_Definition;

typedef struct animation {
    size_t animation_defination_id;
    f32 current_frame_time;
    u8 current_frame_index;
    bool does_loop;
    bool is_active;
    bool is_flipped;
} Animation;

void animation_init(void);
size_t animation_definition_create(Sprite_Sheet *sprite_sheet, f32 duration, u8 row, u8 *columns, u8 frame_count);
size_t animation_create(size_t animation_definition_id, bool does_loop);
void animation_destory(size_t id);
Animation *animation_get(size_t id);
void animation_update(f32 dt);
void animation_render(Animation *animation, vec2 position, vec4 color, u32 texture_slots[8]);
