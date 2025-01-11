/* Copyright 2023-2025 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include "main.h"

#include "tile.h"
#include "entity.h"

// Level size (in tiles)
#define LEVEL_W (16)
#define LEVEL_H (16)
#define LEVEL_SIZE (LEVEL_W * LEVEL_H)

// Tile size: 3 = 8x8, 4 = 16x16, 5 = 32x32
#define LEVEL_TILE_SIZE (4)

typedef u8 level_EntityID;
#define LEVEL_ENTITY_LIMIT (255)

// An invalid entity ID.
#define LEVEL_NO_ENTITY (LEVEL_ENTITY_LIMIT)

#define LEVEL_SOLID_ENTITIES_IN_TILE (4)

#define LEVEL_OBSTACLE_TYPES 3

struct Level {
    enum tile_TypeID tiles[LEVEL_SIZE];
    u8 data[LEVEL_SIZE];

    struct entity_Data entities[LEVEL_ENTITY_LIMIT];

    level_EntityID
    solid_entities[LEVEL_SIZE][LEVEL_SOLID_ENTITIES_IN_TILE];

    const struct level_Metadata *metadata;

    bool editing;
    u8 letters_to_deliver;

    // visual properties
    struct {
        i32 x;
        i32 y;
    } offset;

    bool shake;
    u8 shake_time;
};

struct level_Metadata {
    const u8 *tile_data;

    struct {
        u8 w : 4;
        u8 h : 4;
    } size;

    struct {
        u8 x : 4;
        u8 y : 4;
    } spawn;

    u8 obstacles[LEVEL_OBSTACLE_TYPES];

    // list terminated by (0, 0)
    struct {
        u8 x : 4;
        u8 y : 4;
    } mailboxes[9];

    // list terminated by (0, 0)
    struct {
        u8 x;
        u8 y;
    } grass[8];

    // list terminated by (0, 0)
    struct {
        u8 x : 4;
        u8 y : 4;

        bool lower;
    } houses[9];

    bool tutorial;
    bool tutorial_bubbles;
    u8 tutorial_text;
};

extern void level_tick(struct Level *level);
extern void level_draw(struct Level *level);

extern void level_load(struct Level *level,
                       const struct level_Metadata *metadata);

// === Tile functions ===

INLINE enum tile_TypeID level_get_tile(struct Level *level,
                                       i32 x, i32 y) {
    if(x >= 0 && y >= 0 && x < LEVEL_W && y < LEVEL_H)
        return level->tiles[x + y * LEVEL_W];
    return TILE_INVALID;
}

INLINE void level_set_tile(struct Level *level, i32 x, i32 y,
                           enum tile_TypeID id) {
    if(x >= 0 && y >= 0 && x < LEVEL_W && y < LEVEL_H)
        level->tiles[x + y * LEVEL_W] = id;
}

INLINE u8 level_get_data(struct Level *level, i32 x, i32 y) {
    if(x >= 0 && y >= 0 && x < LEVEL_W && y < LEVEL_H)
        return level->data[x + y * LEVEL_W];
    return 0;
}

INLINE void level_set_data(struct Level *level, i32 x, i32 y, u8 data) {
    if(x >= 0 && y >= 0 && x < LEVEL_W && y < LEVEL_H)
        level->data[x + y * LEVEL_W] = data;
}

// === Entity functions ===

// returns an available entity ID, or an invalid ID if there is none
extern level_EntityID level_new_entity(struct Level *level);

// Finalizes the entity having the given ID, setting its type and
// clearing the 'should_remove' bit. Set the entity's properties
// (such as x and y coordinates) *before* calling this function.
extern void level_add_entity(struct Level *level,
                             enum entity_TypeID type,
                             level_EntityID id);

// === Levels ===

#define LEVEL_COUNT 17
extern const struct level_Metadata level_metadata[LEVEL_COUNT];

// ===== ===== =====

extern bool level_add_player(struct Level *level);
extern bool level_add_mailbox(struct Level *level, u32 xt, u32 yt);

extern bool level_add_edit_cursor(struct Level *level);
extern bool level_add_edit_sidebar(struct Level *level);

extern bool level_add_decor_grass(struct Level *level, u32 x, u32 y);
extern bool level_add_decor_house(struct Level *level, u32 xt, u32 yt,
                                  bool lower);

extern bool level_add_particle_block(struct Level *level, u32 xt, u32 yt,
                                     enum tile_TypeID block);
extern bool level_add_particle_step(struct Level *level, u32 xt, u32 yt);
extern bool level_add_particle_platform(struct Level *level,
                                        u32 xt, u32 yt);
extern bool level_add_particle_tutorial_bubble(struct Level *level,
                                               u32 xt, u32 yt,
                                               enum tile_TypeID tile);
