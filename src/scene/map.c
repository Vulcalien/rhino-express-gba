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

#include <gba/input.h>

#define PAGE_COUNT 3
static u8 first_level_in_pages[PAGE_COUNT + 1] = {
    0, 6, 11, 17
};

static i8 page;
static i8 level;

static void map_init(void *data) {
    // TODO set level and page
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
        // check if the player tried to move to a non-existing page
        if(page < 0)
            page = 0;
        if(page >= PAGE_COUNT)
            page = PAGE_COUNT - 1;

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

    // check if the player tried to move to a non-existing level
    if(level < 0)
        level = 0;
    if(level > levels_cleared)
        level = levels_cleared;

    // adjust the selected page
    if(level < first_level_in_pages[page])
        page--;
    if(level >= first_level_in_pages[page + 1])
        page++;
}

static void map_draw(void) {
    // TODO
}

const struct Scene scene_map = {
    .init = map_init,
    .tick = map_tick,
    .draw = map_draw
};
