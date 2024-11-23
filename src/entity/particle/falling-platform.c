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

#define LIFETIME 20

struct falling_platform_Data {
    u8 age;
    u8 affine_parameter;

    u8 unused[14];
};
ASSERT_SIZE(struct falling_platform_Data, ENTITY_EXTRA_DATA_SIZE);

IWRAM_SECTION
static void falling_platform_tick(struct Level *level,
                                  struct entity_Data *data) {
    struct falling_platform_Data *platform_data =
        (struct falling_platform_Data *) &data->data;

    platform_data->age++;
    if(platform_data->age >= LIFETIME)
        data->should_remove = true;
}

IWRAM_SECTION
static u32 falling_platform_draw(struct Level *level,
                                 struct entity_Data *data,
                                 i32 x, i32 y, u32 used_sprites) {
    struct falling_platform_Data *platform_data =
        (struct falling_platform_Data *) &data->data;

    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 8,
        .y = y - 8,

        .size = SPRITE_SIZE_16x16,

        .tile = 256 + 24,
        .colors = 1,

        .affine = 1,
        .affine_parameter = platform_data->affine_parameter
    });

    const u32 scale = 0x4000 - 0x3fff * platform_data->age / LIFETIME;
    sprite_affine(platform_data->affine_parameter, (i16 [4]) {
        256 * 0x4000 / scale, 0,
        0, 256 * 0x4000 / scale
    });

    return 1;
}

const struct entity_Type entity_particle_falling_platform = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = falling_platform_tick,
    .draw = falling_platform_draw
};

bool level_add_particle_platform(struct Level *level, u32 xt, u32 yt) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    struct entity_Data *data = &level->entities[id];

    data->x = (xt << LEVEL_TILE_SIZE) + 8;
    data->y = (yt << LEVEL_TILE_SIZE) + 8;

    struct falling_platform_Data *platform_data =
        (struct falling_platform_Data *) &data->data;

    platform_data->age = 0;

    // assign affine parameter in range [2, 9]
    static u8 last_affine_parameter = 0;
    platform_data->affine_parameter = 2 + last_affine_parameter;
    last_affine_parameter = (last_affine_parameter + 1) % 8;

    level_add_entity(level, ENTITY_PARTICLE_FALLING_PLATFORM, id);
    return true;
}
