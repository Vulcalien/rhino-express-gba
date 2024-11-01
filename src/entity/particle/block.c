/* Copyright 2024 Vulcalien
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
#include <random.h>

#include "level.h"

#define ANIMATION_PHASES 6

struct particle_block_Data {
    u16 time;
    u8 phase; // animation phase (i.e. sprite)

    u8 spriteset;

    u8 unused[12];
};

static_assert(
    sizeof(struct particle_block_Data) == ENTITY_EXTRA_DATA_SIZE,
    "struct particle_block_Data is of wrong size"
);

IWRAM_SECTION
static void block_tick(struct Level *level, struct entity_Data *data) {
    struct particle_block_Data *particle_data =
        (struct particle_block_Data *) &data->data;

    particle_data->time++;
    if(random(5) == 0 || particle_data->time == 8) {
        particle_data->time = 0;
        particle_data->phase++;

        if(particle_data->phase >= ANIMATION_PHASES)
            data->should_remove = true;
    }
}

IWRAM_SECTION
static u32 block_draw(struct Level *level, struct entity_Data *data,
                      i32 x, i32 y, u32 used_sprites) {
    struct particle_block_Data *particle_data =
        (struct particle_block_Data *) &data->data;

    u32 phase = particle_data->phase;

    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 4,
        .y = y - 4,

        .size = SPRITE_SIZE_8x8,

        .tile = 256 + 64 + particle_data->spriteset * 8 + phase,
        .colors = 1
    });
    return 1;
}

const struct entity_Type entity_particle_block = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = block_tick,
    .draw = block_draw
};

bool level_add_particle_block(struct Level *level, u32 xt, u32 yt,
                              enum tile_TypeID block) {
    for(u32 i = 0; i < 3; i++) {
        level_EntityID id = level_new_entity(level);
        if(id == LEVEL_NO_ENTITY)
            return false;

        // set generic entity data
        struct entity_Data *data = &level->entities[id];

        data->x = (xt << LEVEL_TILE_SIZE) + 8 + (random(9) - 4);
        data->y = (yt << LEVEL_TILE_SIZE) + 8 + (random(9) - 4);

        // set specific particle data
        struct particle_block_Data *particle_data =
            (struct particle_block_Data *) &data->data;

        switch(block) {
            case TILE_WOOD:
                particle_data->spriteset = 0;
                break;
            case TILE_ROCK:
                particle_data->spriteset = 1;
                break;
            case TILE_WATER:
                particle_data->spriteset = 2;
                break;
            default:
                particle_data->spriteset = 0;
                break;
        }
        particle_data->time = 0;
        particle_data->phase = 0;

        level_add_entity(level, ENTITY_PARTICLE_BLOCK, id);
    }
    return true;
}
