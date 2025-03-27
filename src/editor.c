/* Copyright 2024-2025 Vulcalien
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
#include "editor.h"

#include <gba/display.h>
#include <gba/sprite.h>
#include <gba/input.h>
#include <random.h>
#include <memory.h>
#include <math.h>

#include "level.h"
#include "tile.h"
#include "entity.h"
#include "crosshair.h"
#include "music.h"
#include "sfx.h"

#include "res/img/level-sidebar.c"

#define SIDEBAR_TIME_MAX 55

i32 editor_xt;
i32 editor_yt;

static u8 obstacles[LEVEL_OBSTACLE_TYPES];

static i32 selected;
static bool flip;

static struct {
    bool present;

    i32 time;
    i32 step;
} sidebar;

// keeps track of which tiles have been edited
static bool tile_modified[LEVEL_SIZE];

static bool any_obstacle_left(void) {
    for(u32 i = 0; i < LEVEL_OBSTACLE_TYPES; i++)
        if(obstacles[i] > 0)
            return true;
    return false;
}

static void reset_obstacles(const u8 data[LEVEL_OBSTACLE_TYPES]) {
    for(u32 i = 0; i < LEVEL_OBSTACLE_TYPES; i++)
        obstacles[i] = data[i];
}

static void switch_item(i32 step) {
    // if there is no obstacle left, do nothing
    if(!any_obstacle_left())
        return;

    do {
        selected += step;

        if(selected < 0)
            selected = LEVEL_OBSTACLE_TYPES - 1;
        if(selected >= LEVEL_OBSTACLE_TYPES)
            selected = 0;
    } while(obstacles[selected] == 0);

    flip = random(2);
}

void editor_init(struct Level *level) {
    const struct level_Metadata *metadata = level->metadata;

    reset_obstacles(metadata->obstacles);
    level->editing = any_obstacle_left();

    // if editing tutorial is delayed, do not edit on the first attempt
    if(metadata->delay_editing_tutorial && level->attempts == 0)
        level->editing = false;

    // set cursor position to the level's center
    editor_xt = metadata->size.w / 2;
    editor_yt = metadata->size.h / 2;

    // select the first available obstacle
    selected = LEVEL_OBSTACLE_TYPES - 1;
    switch_item(+1);

    // initialize sidebar
    sidebar.present = level->editing;
    sidebar.time = 0;
    sidebar.step = +1;

    // load edit sidebar
    memory_copy_32(
        (vu8 *) display_charblock(4) + 128 * 32,
        level_sidebar,
        4 * 8 * 32
    );

    // clear 'tile_modified'
    memory_clear(tile_modified, sizeof(tile_modified));

    // play music
    if(level->editing)
        MUSIC_PLAY(music_editing);
}

static inline bool try_to_place(struct Level *level) {
    const i32 xt = editor_xt;
    const i32 yt = editor_yt;

    if(level_get_tile(level, xt, yt) != TILE_PLATFORM)
        return false;

    // check if there is a solid entity (player or mailbox) in the tile
    for(u32 i = 0; i < LEVEL_SOLID_ENTITIES_IN_TILE; i++) {
        const u32 tile = xt + yt * LEVEL_W;
        const level_EntityID id = level->solid_entities[tile][i];

        if(id < LEVEL_ENTITY_LIMIT)
            return false;
    }

    enum tile_TypeID tile = TILE_WOOD + selected;
    obstacles[selected]--;

    // place tile and set its data
    level_set_tile(level, xt, yt, tile);
    level_set_data(
        level, xt, yt,
        1    << 0 | // platform
        flip << 1   // flip
    );
    tile_modified[xt + yt * LEVEL_W] = true;

    if(obstacles[selected] == 0) {
        if(any_obstacle_left())
            switch_item(+1);
        else
            level->editing = false;
    }

    level_add_particle_block(level, xt, yt, tile);
    SFX_PLAY(sfx_obstacle_placed);
    return true;
}

static inline void remove_placed_obstacles(struct Level *level) {
    for(u32 yt = 0; yt < level->metadata->size.h; yt++) {
        for(u32 xt = 0; xt < level->metadata->size.w; xt++) {
            if(!tile_modified[xt + yt * LEVEL_W])
                continue;

            // add block particle
            const enum tile_TypeID tile = level_get_tile(level, xt, yt);
            level_add_particle_block(level, xt, yt, tile);

            // replace with platform tile
            level_set_tile(level, xt, yt, TILE_PLATFORM);
            tile_modified[xt + yt * LEVEL_W] = false;
        }
    }
    reset_obstacles(level->metadata->obstacles);
}

static inline void move_cursor(struct Level *level) {
    i32 xt = editor_xt;
    i32 yt = editor_yt;

    if(input_pressed(KEY_LEFT )) xt--;
    if(input_pressed(KEY_RIGHT)) xt++;
    if(input_pressed(KEY_UP   )) yt--;
    if(input_pressed(KEY_DOWN )) yt++;

    xt = math_clip(xt, 0, level->metadata->size.w - 1);
    yt = math_clip(yt, 0, level->metadata->size.h - 1);

    editor_xt = xt;
    editor_yt = yt;
}

static inline void animate_sidebar(struct Level *level) {
    if(!level->editing)
        sidebar.step = -1;

    sidebar.time += sidebar.step;
    switch(sidebar.step) {
        case +1:
            // sidebar reached its point: stop moving
            if(sidebar.time == SIDEBAR_TIME_MAX)
                sidebar.step = 0;
            break;
        case -1:
            if(sidebar.time < 0)
                sidebar.present = false;
            break;
    }
}

IWRAM_SECTION
void editor_tick(struct Level *level) {
    if(level->editing) {
        if(input_pressed(KEY_SELECT))
            level->editing = false;

        // change selected obstacle if L or R pressed
        i32 switch_step = 0;
        switch_step -= input_pressed(KEY_L);
        switch_step += input_pressed(KEY_R);
        if(switch_step != 0)
            switch_item(switch_step);

        if(input_pressed(KEY_A))
            try_to_place(level);

        if(input_pressed(KEY_B))
            remove_placed_obstacles(level);

        move_cursor(level);
    }

    if(sidebar.present)
        animate_sidebar(level);
}

static inline void draw_cursor(struct Level *level, u32 *used_sprites) {
    const i32 x = (editor_xt << LEVEL_TILE_SIZE) - level->offset.x + 8;
    const i32 y = (editor_yt << LEVEL_TILE_SIZE) - level->offset.y + 8;

    sprite_config((*used_sprites)++, &(struct Sprite) {
        .x = x - 8,
        .y = y - 8,

        .size = SPRITE_SIZE_16x16,
        .flip = flip,

        .tile = 32 + selected * 4,
        .palette = (selected == 0 ? 1 : 0)
    });

    *used_sprites += crosshair_draw(*used_sprites, x, y);
}

static inline void draw_sidebar(struct Level *level, u32 *used_sprites) {
    i32 t = sidebar.time * math_brad(120) / SIDEBAR_TIME_MAX;

    const i32 x = -32 + math_sin(t) * 48 / 0x4000;
    const i32 y = (DISPLAY_H - 64) / 2;

    // draw resources
    for(u32 i = 0; i < LEVEL_OBSTACLE_TYPES; i++) {
        const u32 count = obstacles[i];
        if(count == 0)
            continue;

        // resource count
        if(count > 1) {
            sprite_config((*used_sprites)++, &(struct Sprite) {
                .x = x + 6,
                .y = y + 13 + i * 16,

                .size = SPRITE_SIZE_8x8,

                .tile = 44 + (count - 2),
                .palette = 0
            });
        }

        // resource image
        sprite_config((*used_sprites)++, &(struct Sprite) {
            .x = x + 8,
            .y = y + 8 + i * 16,

            .size = SPRITE_SIZE_16x16,

            .tile = 32 + i * 4,
            .palette = (i == 0 ? 1 : 0)
        });
    }

    // draw sidebar background
    sprite_config((*used_sprites)++, &(struct Sprite) {
        .x = x,
        .y = y,

        .size = SPRITE_SIZE_32x64,

        .tile = 128,
        .palette = 0
    });
}

IWRAM_SECTION
void editor_draw(struct Level *level, u32 *used_sprites) {
    if(level->editing)
        draw_cursor(level, used_sprites);
    if(sidebar.present)
        draw_sidebar(level, used_sprites);
}
