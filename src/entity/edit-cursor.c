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

#include <gba/input.h>
#include <gba/sprite.h>
#include <random.h>

#include "level.h"
#include "tile.h"
#include "crosshair.h"
#include "music.h"
#include "sfx.h"

struct cursor_Data {
    i8 selected;

    bool flip;

    u8 unused[14];
};
ASSERT_SIZE(struct cursor_Data, ENTITY_EXTRA_DATA_SIZE);

static inline bool any_obstacle_left(struct Level *level) {
    for(u32 i = 0; i < LEVEL_OBSTACLE_TYPES; i++)
        if(level->obstacles_to_add[i] != 0)
            return true;
    return false;
}

static inline void switch_item(struct Level *level,
                               struct entity_Data *data, i32 step) {
    struct cursor_Data *cursor_data = (struct cursor_Data *) &data->data;

    // if there is no obstacle left, do nothing
    if(!any_obstacle_left(level))
        return;

    do {
        cursor_data->selected += step;

        if(cursor_data->selected < 0)
            cursor_data->selected = LEVEL_OBSTACLE_TYPES - 1;
        else if(cursor_data->selected >= LEVEL_OBSTACLE_TYPES)
            cursor_data->selected = 0;
    } while(level->obstacles_to_add[cursor_data->selected] == 0);

    cursor_data->flip = random(2);
}

static inline bool try_to_place(struct Level *level,
                                struct entity_Data *data,
                                i32 xt, i32 yt) {
    struct cursor_Data *cursor_data = (struct cursor_Data *) &data->data;

    if(level_get_tile(level, xt, yt) != TILE_PLATFORM)
        return false;

    // check if there is a solid entity (player or mailbox) in the tile
    for(u32 i = 0; i < LEVEL_SOLID_ENTITIES_IN_TILE; i++) {
        const u32 tile = xt + yt * LEVEL_W;
        const level_EntityID id = level->solid_entities[tile][i];

        if(id < LEVEL_ENTITY_LIMIT)
            return false;
    }

    enum tile_TypeID tile = TILE_WOOD + cursor_data->selected;
    level->obstacles_to_add[cursor_data->selected]--;

    // place tile and set its data
    level_set_tile(level, xt, yt, tile);
    level_set_data(
        level, xt, yt,
        1                 << 0 | // platform
        cursor_data->flip << 1   // flip
    );

    if(level->obstacles_to_add[cursor_data->selected] == 0)
        switch_item(level, data, +1);

    level_add_particle_block(level, xt, yt, tile);
    SFX_PLAY(sfx_obstacle_placed);
    return true;
}

static inline void move_cursor(struct Level *level) {
    i32 xt = level->editor.xt;
    i32 yt = level->editor.yt;

    if(input_pressed(KEY_LEFT )) xt--;
    if(input_pressed(KEY_RIGHT)) xt++;
    if(input_pressed(KEY_UP   )) yt--;
    if(input_pressed(KEY_DOWN )) yt++;

    // check if 'xt' is out of bounds
    if(xt < 0)
        xt = 0;
    if(xt >= level->metadata->size.w)
        xt = level->metadata->size.w - 1;

    // check if 'yt' is out of bounds
    if(yt < 0)
        yt = 0;
    if(yt >= level->metadata->size.h)
        yt = level->metadata->size.h - 1;

    level->editor.xt = xt;
    level->editor.yt = yt;
}

IWRAM_SECTION
static void cursor_tick(struct Level *level,
                        struct entity_Data *data) {
    if(!any_obstacle_left(level) || input_pressed(KEY_START)) {
        // exit editing mode
        level->is_editing = false;
        MUSIC_PLAY(music_game);

        data->should_remove = true;
        return;
    }

    // change selected obstacle if L or R pressed
    {
        i32 switch_step = 0;
        switch_step -= input_pressed(KEY_L);
        switch_step += input_pressed(KEY_R);
        if(switch_step != 0)
            switch_item(level, data, switch_step);
    }

    if(input_pressed(KEY_A))
        try_to_place(level, data, level->editor.xt, level->editor.yt);

    move_cursor(level);
}

IWRAM_SECTION
static u32 cursor_draw(struct Level *level,
                       struct entity_Data *data,
                       i32 x, i32 y, u32 used_sprites) {
    struct cursor_Data *cursor_data = (struct cursor_Data *) &data->data;

    x = (level->editor.xt << LEVEL_TILE_SIZE) - level->offset.x + 8;
    y = (level->editor.yt << LEVEL_TILE_SIZE) - level->offset.y + 8;

    if(!any_obstacle_left(level))
        return 0;

    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 8,
        .y = y - 8,

        .size = SPRITE_SIZE_16x16,
        .flip = cursor_data->flip,

        .tile = 32 + cursor_data->selected * 4,
        .palette = (cursor_data->selected == 0)
    });

    return 1 + crosshair_draw(used_sprites, x, y);
}

const struct entity_Type entity_edit_cursor = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = cursor_tick,
    .draw = cursor_draw
};

bool level_add_edit_cursor(struct Level *level) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    struct entity_Data *data = &level->entities[id];
    data->x = data->y = 0;

    struct cursor_Data *cursor_data = (struct cursor_Data *) &data->data;

    // select the first available obstacle
    cursor_data->selected = LEVEL_OBSTACLE_TYPES - 1;
    switch_item(level, data, +1);

    level_add_entity(level, ENTITY_EDIT_CURSOR, id);
    return true;
}
