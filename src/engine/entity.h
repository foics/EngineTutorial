#pragma once

#include <stdbool.h>
#include <linmath.h>

#include "physics.h"
#include "types.h"

typedef struct entity {
    size_t body_id;
    size_t animation_id;
    bool is_active;
} Entity;

void entity_init(void);
size_t entity_create(vec2 position, vec2 size, vec2 velocity, u8 collision_layer, u8 collision_mask, bool is_kinematic, On_Hit on_hit, On_Hit_Static on_hit_static);
Entity *entity_get(size_t id);
size_t entity_count();
