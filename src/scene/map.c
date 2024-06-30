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
#include <gba/sprite.h>
#include <gba/input.h>
#include <gba/dma.h>
#include <memory.h>
#include <math.h>

#include "level.h"
#include "screen.h"

#define PAGE_COUNT 3
static u8 first_level_in_pages[PAGE_COUNT + 1] = {
    0, 6, 11, 17
};

static u16 draw_offset;

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
    if(input_pressed(KEY_A) || input_pressed(KEY_START))
        if(level < LEVEL_COUNT)
            scene_set(&scene_game, true, (u32 []) { level });
}

#include "../res/map.c"

IWRAM_SECTION
static void map_draw(void) {
    // draw level selection buttons
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

    for(u32 i = 0; i < levels_cleared; i++) {
        i32 draw_x = level_buttons[i].x - draw_offset;
        i32 draw_y = level_buttons[i].y;

        if(draw_x < -8 || draw_x > 248) {
            sprite_hide(i);
            continue;
        }

        struct Sprite sprite = {
            .x = draw_x - 8,
            .y = draw_y - 8,

            .size = 1,

            .tile = 512 + 0, // DEBUG
            .color_mode = 1
        };
        sprite_config(i, &sprite);
    }

    // draw bitmap
    vu8 *raster = (vu8 *) display_get_raster(0);
    for(u32 y = 0; y < 160; y++) {
        dma_config(DMA3, &(struct DMA) {
            .chunk = DMA_CHUNK_32_BIT
        });
        dma_transfer(
            DMA3,
            (vu32 *) &raster[y * 240],
            (vu32 *) &map[draw_offset + y * 240 * 4],
            240 / 4
        );
    }
}

const struct Scene scene_map = {
    .init = map_init,
    .tick = map_tick,
    .draw = map_draw
};
