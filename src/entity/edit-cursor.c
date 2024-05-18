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

#include "level.h"
#include "tile.h"
#include "music.h"
#include "input.h"
#include "sprite.h"

static inline bool any_obstacle_left(struct Level *level,
                                     struct entity_Data *data) {
    return level->obstacles_to_add.wood  != 0 ||
           level->obstacles_to_add.rock  != 0 ||
           level->obstacles_to_add.water != 0;
}

static inline bool try_to_place(struct Level *level,
                                struct entity_Data *data,
                                i32 xt, i32 yt) {
    if(level_get_tile(level, xt, yt) != TILE_PLATFORM)
        return false;

    // check if there is a solid entity (player or mailbox) in the tile
    for(u32 i = 0; i < LEVEL_SOLID_ENTITIES_IN_TILE; i++) {
        const u32 tile = xt + yt * LEVEL_W;
        const level_EntityID id = level->solid_entities[tile][i];

        if(id < LEVEL_ENTITY_LIMIT)
            return false;
    }

    // DEBUG
    enum tile_TypeID obstacle = TILE_WOOD; // DEBUG
    level->obstacles_to_add.wood--;

    level_set_tile(level, xt, yt, obstacle);
    return true;
}

static inline void move_cursor(struct Level *level,
                               struct entity_Data *data,
                               i32 xm, i32 ym) {
    if(xm == 0 && ym == 0)
        return;

    i32 new_x = data->x + (xm << LEVEL_TILE_SIZE);
    i32 new_y = data->y + (ym << LEVEL_TILE_SIZE);

    // if the cursor is not going outside the level, move it
    if(new_x >= 0 && new_x < (LEVEL_W << LEVEL_TILE_SIZE) &&
       new_y >= 0 && new_y < (LEVEL_H << LEVEL_TILE_SIZE)) {
        data->x = new_x;
        data->y = new_y;
    }
}

IWRAM_SECTION
static void cursor_tick(struct Level *level,
                        struct entity_Data *data) {
    if(!any_obstacle_left(level, data) || input_pressed(KEY_START)) {
        // exit editing mode
        level->is_editing = false;
        SOUND_PLAY(music_game, true, SOUND_CHANNEL_B);

        data->should_remove = true;
        return;
    }

    const i32 xt = data->x >> LEVEL_TILE_SIZE;
    const i32 yt = data->y >> LEVEL_TILE_SIZE;

    if(input_pressed(KEY_A))
        try_to_place(level, data, xt, yt);

    // cursor movement
    i32 xm = 0;
    i32 ym = 0;

    if(input_pressed(KEY_LEFT))  xm--;
    if(input_pressed(KEY_RIGHT)) xm++;
    if(input_pressed(KEY_UP))    ym--;
    if(input_pressed(KEY_DOWN))  ym++;

    move_cursor(level, data, xm, ym);
}

IWRAM_SECTION
static u32 cursor_draw(struct Level *level,
                       struct entity_Data *data,
                       i32 x, i32 y, u32 used_sprites) {
    struct Sprite sprite = {
        .x = x - 8,
        .y = y - 8,

        .size = 1,

        .tile = (20) * 2, // TODO use sprite of selected obstacle
        .color_mode = 1,

        // TODO .mode = 1, // semi-transparent mode
    };
    sprite_config(used_sprites++, &sprite);

    return 1;
}

const struct entity_Type entity_edit_cursor = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = cursor_tick,
    .draw = cursor_draw
};

bool level_add_edit_cursor(struct Level *level, u32 xt, u32 yt) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    // set generic entity data
    struct entity_Data *data = &level->entities[id];

    data->x = (xt << LEVEL_TILE_SIZE) + 8;
    data->y = (yt << LEVEL_TILE_SIZE) + 8;

    level_add_entity(level, ENTITY_EDIT_CURSOR, id);
    return true;
}
