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

#include "level.h"

#define SIZES 3

struct step_Data {
    i8 size;

    u8 unused[15];
};
ASSERT_SIZE(struct step_Data, ENTITY_EXTRA_SIZE);

IWRAM_SECTION
static void step_tick(struct Level *level, struct entity_Data *data) {
    struct step_Data *step_data = (struct step_Data *) &data->extra;

    // chance of not shrinking: bigger particles shrink faster
    const u32 chance = 9 - step_data->size * 3;

    if(random(chance) == 0) {
        step_data->size--;
        if(step_data->size < 0)
            data->should_remove = true;
    }
}

IWRAM_SECTION
static u32 step_draw(struct Level *level, struct entity_Data *data,
                     i32 x, i32 y, u32 used_sprites) {
    struct step_Data *step_data = (struct step_Data *) &data->extra;

    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 4,
        .y = y - 4,

        .size = SPRITE_SIZE_8x8,

        .tile = 88 + step_data->size,
        .palette = 0
    });
    return 1;
}

const struct entity_Type entity_particle_step = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = step_tick,
    .draw = step_draw
};

bool level_add_particle_step(struct Level *level, u32 xt, u32 yt) {
    const u32 count = 4 + random(3); // 4-7 particles

    for(u32 i = 0; i < count; i++) {
        level_EntityID id = level_new_entity(level);
        if(id == LEVEL_NO_ENTITY)
            return false;

        struct entity_Data *data = &level->entities[id];
        data->x = (xt << LEVEL_TILE_SIZE) + 8 + (random(7) - 3);
        data->y = (yt << LEVEL_TILE_SIZE) + 8 + (random(7) - 3);

        struct step_Data *step_data = (struct step_Data *) &data->extra;
        step_data->size = (SIZES - 1) - random(3);

        level_add_entity(level, ENTITY_PARTICLE_STEP, id);
    }
    return true;
}
