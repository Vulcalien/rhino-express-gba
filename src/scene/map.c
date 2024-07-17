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

#define PAGE_COUNT 3
static u8 first_level_in_pages[PAGE_COUNT + 1] = {
    0, 6, 11, 17
};

static u16 draw_offset; // this value should always be a multiple of two

static i8 page;
static i8 level;

static void map_init(void *data) {
    if(data) {
        bool has_cleared_level = *((bool *) data);
        if(has_cleared_level && level == levels_cleared)
            levels_cleared++;
    }

    // TODO set level and page

    draw_offset = page * 240;

    screen_mode_4();

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
    if(input_pressed(KEY_LEFT))
        level--;
    if(input_pressed(KEY_RIGHT))
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

#include "../res/img/map.c"
#include "../res/img/level-buttons.c"

IWRAM_SECTION
static void map_draw(void) {
    struct {
        i16 x;
        i16 y;
    } level_buttons[LEVEL_COUNT] = {
        { 75,  39  },
        { 75,  79  },
        { 91,  123 },
        { 131, 103 },
        { 143, 55  },
        { 175, 87  },

        { 315, 37  },
        { 323, 89  },
        { 363, 125 },
        { 391, 85  },
        { 395, 41  },

        { 547, 36  },
        { 579, 60  },
        { 555, 92  },
        { 587, 128 },
        { 622, 96  },
        { 631, 52  }
    };

    const u32 crosshair_sprites = 0; // ID of the first crosshair sprite

    // draw bitmap
    vu8 *raster = (vu8 *) display_get_raster(0);
    for(u32 y = 0; y < 160; y++) {
        // 'draw_offset' is always a multiple of two,
        // so 16-bit chunks work well
        dma_config(DMA3, &(struct DMA) {
            .chunk = DMA_CHUNK_16_BIT
        });
        dma_transfer(
            DMA3,
            (vu16 *) &raster[y * 240],
            (vu16 *) &map[draw_offset + y * 240 * 4],
            240 / 2
        );
    }

    // draw level selection buttons
    // FIXME the buttons are a few pixels off
    for(u32 i = 0; i < math_min(levels_cleared + 1, LEVEL_COUNT); i++) {
        // calculate the button's center
        const i32 xc = level_buttons[i].x - draw_offset;
        const i32 yc = level_buttons[i].y;

        // draw the crosshair
        if(i == level)
            crosshair_draw(crosshair_sprites, xc, yc);

        // calculate the top-left corner
        const i32 corner_x = xc- 8;
        const i32 corner_y = yc - 8;

        if(corner_x < -16 || corner_x >= 240)
            continue;

        const i32 x0 = math_max(0, -corner_x);
        const i32 x1 = math_min(16, 240 - corner_x);

        for(u32 y = 0; y < 16; y++) {
            const u32 ypix = corner_y + y;

            for(u32 x = x0; x < x1; x++) {
                const u32 xpix = corner_x + x;

                const u8 pixel = level_button_images[
                    (x + (i == level) * 16) + (y + i * 16) * 32
                ];

                vu16 *pixels = (vu16 *) &raster[
                    (xpix & ~1) + ypix * 240
                ];
                if(xpix & 1)
                    *pixels = (*pixels & 0x00ff) | pixel << 8;
                else
                    *pixels = (*pixels & 0xff00) | pixel << 0;
            }
        }
    }

    if(level == LEVEL_COUNT)
        sprite_hide_range(crosshair_sprites, crosshair_sprites + 4);
}

const struct Scene scene_map = {
    .init = map_init,
    .tick = map_tick,
    .draw = map_draw
};
