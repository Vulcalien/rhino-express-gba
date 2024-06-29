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
#include <memory.h>
#include <math.h>

#define PAGE_COUNT 3
static u8 first_level_in_pages[PAGE_COUNT + 1] = {
    0, 6, 11, 17
};

static u16 draw_offset;

static i8 page;
static i8 level;

static void map_init(void *data) {
    // switch to Video Mode 4
    vsync();
    display_config(&(struct Display) {
        .mode = 4,

        .obj_mapping = 1,

        .enable_bg2 = 1,
        .enable_obj = 1
    });
    display_set_page(0);

    // TODO set level and page

    draw_offset = page * 240;
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
}

#include "../res/map.c"

static void map_draw(void) {
    vu8 *raster = (vu8 *) display_get_raster(0);
    for(u32 y = 0; y < 160; y++) {
        memcpy32(
            (vu32 *) &raster[y * 240],
            (vu32 *) &map[draw_offset + y * 240 * 4],
            240
        );
    }
}

const struct Scene scene_map = {
    .init = map_init,
    .tick = map_tick,
    .draw = map_draw
};
