/* Copyright 2025 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "entity.h"

#include <gba/sprite.h>

#include "level.h"
#include "tile.h"

#define EXPAND_TIME 16          // ticks bubble takes to expand
#define REMAIN_TIME 128         // ticks bubble stays fully scaled
#define SHRINK_TIME EXPAND_TIME // ticks bubble takes to shrink

#define TOTAL_TIME (EXPAND_TIME + REMAIN_TIME + SHRINK_TIME)

struct bubble_Data {
    u16 age;

    u8 obstacle;
    u8 affine_parameter;

    u8 unused[12];
};
ASSERT_SIZE(struct bubble_Data, ENTITY_EXTRA_SIZE);

IWRAM_SECTION
static void bubble_tick(struct Level *level, struct entity_Data *data) {
    struct bubble_Data *bubble_data = (struct bubble_Data *) &data->extra;

    bubble_data->age++;
    if(bubble_data->age > TOTAL_TIME)
        data->should_remove = true;
}

IWRAM_SECTION
static u32 bubble_draw(struct Level *level, struct entity_Data *data,
                       i32 x, i32 y, u32 used_sprites) {
    struct bubble_Data *bubble_data = (struct bubble_Data *) &data->extra;

    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 8,
        .y = y - 24,

        .size = SPRITE_SIZE_16x32,

        .tile = 96 + bubble_data->obstacle * 8,
        .palette = 2,

        .affine = 1,
        .affine_parameter = bubble_data->affine_parameter
    });

    // calculate scale factor based on bubble age
    u32 scale_y;
    if(bubble_data->age < EXPAND_TIME) {
        scale_y = 0x4000 * (1 + bubble_data->age) / EXPAND_TIME;
    } else if(bubble_data->age < EXPAND_TIME + REMAIN_TIME) {
        scale_y = 0x4000;
    } else {
        // calculate scale based on ticks since bubble started shrinking
        u32 shrink_age = bubble_data->age - (EXPAND_TIME + REMAIN_TIME);
        scale_y = 0x4000 - 0x3fff * shrink_age / SHRINK_TIME;
    }

    sprite_affine(bubble_data->affine_parameter, (i16 [4]) {
        256, 0,
        0, 256 * 0x4000 / scale_y
    });

    return 1;
}

const struct entity_Type entity_particle_tutorial_bubble = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = bubble_tick,
    .draw = bubble_draw
};

bool level_add_particle_tutorial_bubble(struct Level *level,
                                        u32 xt, u32 yt,
                                        enum tile_TypeID tile) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    struct entity_Data *data = &level->entities[id];
    data->x = (xt << LEVEL_TILE_SIZE) + 8;
    data->y = (yt << LEVEL_TILE_SIZE) + 8;

    struct bubble_Data *bubble_data = (struct bubble_Data *) &data->extra;
    bubble_data->obstacle = (tile - TILE_WOOD);

    // assign affine parameter in range [10, 17]
    static u8 last_affine_parameter = 0;
    bubble_data->affine_parameter = 10 + last_affine_parameter;
    last_affine_parameter = (last_affine_parameter + 1) % 8;

    level_add_entity(level, ENTITY_PARTICLE_TUTORIAL_BUBBLE, id);
    return true;
}
