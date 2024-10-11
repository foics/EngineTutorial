#pragma once

#include <stdbool.h>
#include <linmath.h>

#include "types.h"

typedef struct hit Hit;
typedef struct body Body;
typedef struct static_body Static_Body;

typedef void (*On_Hit)(Body *self, Body *other, Hit hit);
typedef void (*On_Hit_Static)(Body *self, Static_Body *other, Hit hit);

typedef struct aabb {
    vec2 position;
    vec2 half_size;
} AABB;

typedef struct body {
    AABB aabb;
    vec2 velocity;
    vec2 acceleration;
    On_Hit on_hit;
    On_Hit_Static on_hit_static;
    size_t entity_id;
    u8 collision_layer;
    u8 collision_mask;
    bool is_kinematic;
    bool is_active;
} Body;

typedef struct static_body {
    AABB aabb;
    u8 collision_layer;
} Static_Body;

typedef struct hit {
    size_t other_id;
    f32 time;
    vec2 position;
    vec2 normal;
    bool is_hit;
} Hit;

void physics_init(void);
void physics_update(void);
Body *physics_body_get(size_t index);
size_t physics_body_create(vec2 position, vec2 size, vec2 velocity, u8 collision_layer, u8 collision_mask, bool is_kinematic, On_Hit on_hit, On_Hit_Static on_hit_static, size_t entity_id);
size_t physics_trigger_create(vec2 position, vec2 size, u8 collision_layer, u8 collision_mask, On_Hit on_hit);
Static_Body *physics_static_body_get(size_t index);
size_t physics_static_body_create(vec2 position, vec2 size, u8 collision_layer);
size_t physics_static_body_count();
bool physics_point_intersect_aabb(vec2 point, AABB aabb);
bool physics_aabb_intersect_aabb(AABB a, AABB b);
AABB aabb_minkowski_difference(AABB a, AABB b);
void aabb_penetration_vector(vec2 r, AABB aabb);
void aabb_min_max(vec2 min, vec2 max, AABB aabb);
Hit ray_intersect_aabb(vec2 position, vec2 magnitude, AABB aabb);
void physics_reset(void);
