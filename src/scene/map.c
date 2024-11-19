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
#include "scene.h"

#include <gba/display.h>
#include <gba/input.h>
#include <gba/dma.h>
#include <memory.h>
#include <math.h>

#include "level.h"
#include "screen.h"
#include "crosshair.h"
#include "storage.h"
#include "music.h"

#include "../res/img/map.c"
#include "../res/img/level-buttons.c"

#define PAGE_COUNT 4
static u8 first_level_in_pages[PAGE_COUNT] = {
    0, 6, 11, 17
};

static u16 draw_offset; // this value should always be a multiple of two

static i8 page;
static i8 level;

static void map_init(void *data) {
    if(data) {
        bool has_cleared_level = *((bool *) data);
        if(has_cleared_level && level == levels_cleared) {
            levels_cleared++;
            storage_save();
        }

        // Only the level calls this function passing a non-null 'data'
        // pointer. And since the level plays music, the 'map' music has
        // to be restarted. The 'start' scene, instead, plays the same
        // music as the map, therefore it should not be restarted.
        MUSIC_PLAY(music_map);
    }

    // TODO set level and page

    draw_offset = page * 240;

    screen_mode_4();

    // load level selection button images
    memcpy32(
        (vu8 *) display_charblock(5) + 128 * 64,
        level_button_images,
        68 * 64
    );

    // draw now to prevent showing garbage on the first frame
    scene_map.draw();
}

static inline void validate_bounds_i8(i8 *val, i8 min, i8 max) {
    if(*val < min)
        *val = min;
    if(*val > max)
        *val = max;
}

static inline void update_draw_offset(void) {
    if(draw_offset != page * 240) {
        u32 diff = page * 240 - draw_offset;

        u32 speed;
        if(math_abs(diff) > 140)
            speed = 12;
        else if(math_abs(diff) > 100)
            speed = 10;
        else if(math_abs(diff) > 60)
            speed = 8;
        else if(math_abs(diff) > 20)
            speed = 6;
        else if(math_abs(diff) > 4)
            speed = 4;
        else
            speed = 2;

        draw_offset += speed * math_sign(diff);
    }
}

static void map_tick(void) {
    // 0 = none, -1 = right-to-left, +1 = left-to-right
    i32 page_change_dir = 0;

    // move left/right by one page
    if(input_pressed(KEY_L)) {
        page--;
        page_change_dir = -1;
    }
    if(input_pressed(KEY_R)) {
        page++;
        page_change_dir = +1;
    }

    if(page_change_dir != 0) {
        validate_bounds_i8(&page, 0, PAGE_COUNT - 1);

        // update the selected level based on page change direction
        if(page_change_dir < 0)
            level = first_level_in_pages[page + 1] - 1;
        else
            level = first_level_in_pages[page];
    }

    // move left/right by one level
    if(input_pressed(KEY_LEFT) || input_pressed(KEY_UP))
        level--;
    if(input_pressed(KEY_RIGHT) || input_pressed(KEY_DOWN))
        level++;

    validate_bounds_i8(&level, 0, levels_cleared);

    // adjust the selected page
    if(level < first_level_in_pages[page])
        page--;
    if(level >= first_level_in_pages[page + 1])
        page++;

    update_draw_offset();

    // check if the player has chosen a level
    if(input_pressed(KEY_A) || input_pressed(KEY_START)) {
        if(level < LEVEL_COUNT) {
            // when calling 'scene_transition_to', the data passed is
            // not used immediately (it needs static storage duration)
            static u32 game_scene_arg;
            game_scene_arg = level;

            scene_transition_to(&scene_game, &game_scene_arg);
        }
    }
}

static inline void draw_level_buttons(u32 *used_sprites) {
    // TODO check if these values are correct
    struct {
        i16 x;
        i16 y;
    } level_buttons[LEVEL_COUNT] = {
        { 76,  40  },
        { 76,  80  },
        { 92,  124 },
        { 132, 104 },
        { 144, 56  },
        { 176, 88  },

        { 316, 38  },
        { 324, 90  },
        { 364, 126 },
        { 392, 86  },
        { 396, 42  },

        { 548, 37  },
        { 580, 61  },
        { 556, 93  },
        { 588, 129 },
        { 623, 97  },
        { 632, 53  }
    };

    const u32 buttons = math_min(levels_cleared + 1, LEVEL_COUNT);
    for(u32 i = 0; i < buttons; i++) {
        // calculate the button's top-left corner
        const i32 x = level_buttons[i].x - draw_offset - 8;
        const i32 y = level_buttons[i].y - 8;

        if(x < -16 || x >= 240)
            continue;

        if(i == level) {
            // load the active button image
            memcpy32(
                (vu8 *) display_charblock(5) + 196 * 64,
                level_button_images + (72 + i * 4) * 64,
                4 * 64
            );

            // draw crosshair
            crosshair_draw(*used_sprites, x + 8, y + 8);
            *used_sprites += 4;
        }

        sprite_config((*used_sprites)++, &(struct Sprite) {
            .x = x,
            .y = y,

            .size = SPRITE_SIZE_16x16,

            .tile = 256 + (i == level ? 196 : 128 + i * 4),
            .colors = 1
        });
    }
}

static inline void draw_page_arrows(u32 *used_sprites) {
    // fixed point number: 1 = 0x4000
    // scale = 1.25 + sin(t) / 4   --->   range [1, 1.5]
    i32 scale = 0x5000 + math_sin(tick_count * math_brad(90) / 16) / 4;

    // left arrows
    sprite_affine(0, (i16 [4]) {
        -256 * 0x4000 / scale, 0,
        0, 256 * 0x4000 / scale
    });

    // right arrows
    sprite_affine(1, (i16 [4]) {
        256 * 0x4000 / scale, 0,
        0, 256 * 0x4000 / scale
    });

    for(u32 i = 1; i < PAGE_COUNT * 2 - 1; i++) {
        const i32 y = 24;
        const i32 x = (i / 2) * 240 + 120
                      + (i % 2 ? +1 : -1) * 96
                      - draw_offset;

        if(x < -16 || x >= 240 + 16)
            continue;

        sprite_config((*used_sprites)++, &(struct Sprite) {
            .x = x - 16,
            .y = y - 16,

            .size = SPRITE_SIZE_16x16,

            .tile = 256 + 32,
            .colors = 1,

            .affine = 1,
            .affine_parameter = i % 2,
            .double_size = 1
        });
    }
}

IWRAM_SECTION
static void map_draw(void) {
    // draw sprites
    u32 used_sprites = SCREEN_FOG_PARTICLE_COUNT;

    draw_level_buttons(&used_sprites);
    draw_page_arrows(&used_sprites);

    sprite_hide_range(used_sprites, SPRITE_COUNT);

    // draw bitmap
    vu8 *raster = (vu8 *) display_get_raster(0);
    for(u32 y = 0; y < 160; y++) {
        // 'draw_offset' is always a multiple of two,
        // so 16-bit chunks work well
        dma_config(DMA3, &(struct DMA) { .chunk = DMA_CHUNK_16_BIT });
        dma_transfer(
            DMA3,
            &raster[y * 240],
            &map[draw_offset + y * 240 * 4],
            240 / 2
        );
    }
}

const struct Scene scene_map = {
    .init = map_init,
    .tick = map_tick,
    .draw = map_draw
};
