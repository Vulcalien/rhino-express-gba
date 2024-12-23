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

#include <gba/display.h>
#include <gba/sprite.h>
#include <math.h>

#include "level.h"

#define ANIMATION_TIME 55

struct sidebar_Data {
    i16 animation;
    i8 animation_dir;

    u8 unused[13];
};
ASSERT_SIZE(struct sidebar_Data, ENTITY_EXTRA_DATA_SIZE);

IWRAM_SECTION
static void sidebar_tick(struct Level *level, struct entity_Data *data) {
    struct sidebar_Data *sidebar_data = (struct sidebar_Data *) &data->data;

    if(!level->is_editing)
        sidebar_data->animation_dir = -1;

    sidebar_data->animation += sidebar_data->animation_dir;
    switch(sidebar_data->animation_dir) {
        case +1:
            // sidebar reached its point: stop moving
            if(sidebar_data->animation >= ANIMATION_TIME)
                sidebar_data->animation_dir = 0;
            break;
        case -1:
            // empty sidebar disappeared: remove it
            if(sidebar_data->animation <= 0)
                data->should_remove = true;
            break;
    }
}

IWRAM_SECTION
static u32 sidebar_draw(struct Level *level, struct entity_Data *data,
                        i32 x, i32 y, u32 used_sprites) {
    struct sidebar_Data *sidebar_data = (struct sidebar_Data *) &data->data;

    u32 old_used_sprites = used_sprites;

    // calculate x and y (ignore the entity position)
    {
        i32 t = sidebar_data->animation * math_brad(120) / ANIMATION_TIME;

        x = -32 + math_sin(t) * 48 / 0x4000;
        y = (DISPLAY_H - 64) / 2;
    }

    // draw resources
    for(u32 i = 0; i < LEVEL_OBSTACLE_TYPES; i++) {
        const u32 count = level->obstacles_to_add[i];
        if(count == 0)
            continue;

        // resource count
        if(count > 1) {
            sprite_config(used_sprites++, &(struct Sprite) {
                .x = x + 6,
                .y = y + 13 + i * 16,

                .size = SPRITE_SIZE_8x8,

                .tile = 44 + (count - 2),
                .palette = 0
            });
        }

        // resource image
        sprite_config(used_sprites++, &(struct Sprite) {
            .x = x + 8,
            .y = y + 8 + i * 16,

            .size = SPRITE_SIZE_16x16,

            .tile = 32 + i * 4,
            .palette = (i == 0)
        });
    }

    // draw sidebar background
    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x,
        .y = y,

        .size = SPRITE_SIZE_32x64,

        .tile = 128,
        .palette = 0
    });

    return used_sprites - old_used_sprites;
}

const struct entity_Type entity_edit_sidebar = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = sidebar_tick,
    .draw = sidebar_draw
};

bool level_add_edit_sidebar(struct Level *level) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    struct entity_Data *data = &level->entities[id];
    data->x = data->y = 0;

    struct sidebar_Data *sidebar_data = (struct sidebar_Data *) &data->data;
    sidebar_data->animation = 0;
    sidebar_data->animation_dir = 1;

    level_add_entity(level, ENTITY_EDIT_SIDEBAR, id);
    return true;
}
