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

#include "level.h"

IWRAM_SECTION
static void house_tick(struct Level *level, struct entity_Data *data) {
}

IWRAM_SECTION
static u32 house_draw(struct Level *level, struct entity_Data *data,
                      i32 x, i32 y, u32 used_sprites) {
    i32 xt = data->x >> LEVEL_TILE_SIZE;
    i32 yt = data->y >> LEVEL_TILE_SIZE;

    if(level_get_tile(level, xt, yt) == TILE_HIGH_GROUND)
        y -= 4;

    struct Sprite sprite = {
        .x = x - 8,
        .y = y - 8 - 4,

        .size = 1,

        .tile = 512 + 24,
        .color_mode = 1
    };
    sprite_config(used_sprites++, &sprite);

    return 1;
}

const struct entity_Type entity_decor_house = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = house_tick,
    .draw = house_draw
};

bool level_add_decor_house(struct Level *level, u32 xt, u32 yt,
                           bool lower) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    struct entity_Data *data = &level->entities[id];

    data->x = (xt << LEVEL_TILE_SIZE) + 8;
    data->y = (yt << LEVEL_TILE_SIZE) + 8;

    if(lower)
        data->y += 4;

    level_add_entity(level, ENTITY_DECOR_HOUSE, id);
    return true;
}
