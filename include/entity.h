/* Copyright 2023-2024 Vulcalien
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

enum entity_TypeID {
    ENTITY_PLAYER,
    ENTITY_MAILBOX,

    ENTITY_DECOR_GRASS,
    ENTITY_DECOR_HOUSE,

    ENTITY_PARTICLE_BLOCK,
    ENTITY_PARTICLE_STEP,
    ENTITY_PARTICLE_FALLING_PLATFORM,
    ENTITY_PARTICLE_TUTORIAL_BUBBLE,

    ENTITY_INVALID
};
#define ENTITY_TYPES (ENTITY_INVALID)

// number of extra bytes reserved for private use by entity types
#define ENTITY_EXTRA_SIZE 16

struct entity_Data {
    enum entity_TypeID type;

    bool should_remove;
    u8 solid_id;

    i32 x;
    i32 y;

    u8 extra[ENTITY_EXTRA_SIZE] ALIGNED(4);
};

struct Level;
struct entity_Type {
    // entity radius (width and height)
    u8 xr;
    u8 yr;

    bool is_solid;

    void (*tick)(struct Level *level, struct entity_Data *data);

    // returns how many sprites were used
    u32 (*draw)(struct Level *level, struct entity_Data *data,
                i32 x, i32 y, u32 used_sprites);

    // If defined, this is called when this entity (data) touches
    // another entity (touched_data) while trying to move.
    // If 'true' is returned, this entity should be blocked by the
    // touched entity. If not defined, this is the default behavior.
    bool (*touch_entity)(struct Level *level, struct entity_Data *data,
                         struct entity_Data *touched_data);

    // If defined, this is called when an entity trying to move
    // (touching_data) touches this entity (data).
    // If 'true' is returned, the moving entity should be moved by this
    // entity. If not defined, this is the default behavior.
    bool (*touched_by)(struct Level *level, struct entity_Data *data,
                       struct entity_Data *touching_data);
};

extern const struct entity_Type * const entity_type_list[ENTITY_TYPES];

INLINE bool entity_is_valid(struct entity_Data *data) {
    return (data->type >= 0 && data->type < ENTITY_TYPES);
}

INLINE
const struct entity_Type *entity_get_type(struct entity_Data *data) {
    if(entity_is_valid(data))
        return entity_type_list[data->type];
    return NULL;
}

// returns 'true' if the entity moved by exactly (xm, ym)
extern bool entity_move(struct Level *level, struct entity_Data *data,
                        i32 xm, i32 ym);

INLINE bool entity_intersects(struct entity_Data *data,
                              i32 x0, i32 y0, i32 x1, i32 y1) {
    const struct entity_Type *entity_type = entity_get_type(data);

    return (data->x + entity_type->xr - 1 >= x0) &&
           (data->y + entity_type->yr - 1 >= y0) &&
           (data->x - entity_type->xr     <= x1) &&
           (data->y - entity_type->yr     <= y1);
}

INLINE bool entity_touches(struct entity_Data *data1,
                           struct entity_Data *data2) {
    const struct entity_Type *e2_type = entity_get_type(data2);

    return entity_intersects(
        data1,
        data2->x - e2_type->xr,     data2->y - e2_type->yr,
        data2->x + e2_type->xr - 1, data2->y + e2_type->yr - 1
    );
}

// Entity types
extern const struct entity_Type
    entity_player,
    entity_mailbox,

    entity_decor_grass,
    entity_decor_house,

    entity_particle_block,
    entity_particle_step,
    entity_particle_falling_platform,
    entity_particle_tutorial_bubble;
