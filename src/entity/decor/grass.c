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

IWRAM_SECTION
static void grass_tick(struct Level *level, struct entity_Data *data) {
}

IWRAM_SECTION
static u32 grass_draw(struct Level *level, struct entity_Data *data,
                      i32 x, i32 y, u32 used_sprites) {
    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 4,
        .y = y - 4,

        .size = SPRITE_SIZE_8x8,

        .tile = 48, // TODO
        .palette = 1
    });

    return 1;
}

const struct entity_Type entity_decor_grass = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = grass_tick,
    .draw = grass_draw
};

bool level_add_decor_grass(struct Level *level, u32 x, u32 y) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    struct entity_Data *data = &level->entities[id];
    data->x = x;
    data->y = y;

    level_add_entity(level, ENTITY_DECOR_GRASS, id);
    return true;
}
