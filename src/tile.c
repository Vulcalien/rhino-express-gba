/* Copyright 2024 Vulcalien
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "tile.h"

#include "screen.h"
#include "level.h"

#define DRAW_FUNC(name)\
    IWRAM_SECTION\
    static void name(struct Level *level, i32 xt, i32 yt)

#define GET_LOW(level, xt, yt)\
    (&BG3_TILEMAP[(xt) * 2 + ((yt) * 2) * 32])

#define GET_HIGH(level, xt, yt)\
    (&BG2_TILEMAP[(xt) * 2 + ((yt) * 2) * 32])

#define TILE get_tile_value

ALWAYS_INLINE
static inline u16 get_tile_value(u32 number, u32 flip, u32 palette) {
    return (number  & 0x3ff) << 0  |
           (flip    & 0x03)  << 10 |
           (palette & 0x0f)  << 12;
}

IWRAM_SECTION
static void draw_outer_borders(struct Level *level, i32 xt, i32 yt) {
    vu16 *low = GET_LOW(level, xt, yt);

    bool left  = level_get_tile(level, xt - 1, yt) == TILE_VOID;
    bool right = level_get_tile(level, xt + 1, yt) == TILE_VOID;
    bool up    = level_get_tile(level, xt, yt - 1) == TILE_VOID;
    bool down  = level_get_tile(level, xt, yt + 1) == TILE_VOID;

    bool top_left  = level_get_tile(level, xt - 1, yt - 1) == TILE_VOID;
    bool top_right = level_get_tile(level, xt + 1, yt - 1) == TILE_VOID;

    if(down) {
        low[64] = TILE(16, 0, 0);
        low[65] = TILE(16, 0, 0);

        if(left)
            low[63] = TILE(17, 1, 0);
        if(right)
            low[66] = TILE(17, 0, 0);
    }

    if(left) {
        low[31] = TILE(9, 1, 0);
        if(up) {
            low[-1] = TILE(1, 1, 0);
        } else {
            if(top_left)
                low[-1] = TILE(9, 1, 0);
            else
                low[-1] = TILE(24, 1, 0);
        }
    }

    if(right) {
        low[34] = TILE(9, 0, 0);
        if(up) {
            low[2] = TILE(1, 0, 0);
        } else {
            if(top_right)
                low[2] = TILE(9, 0, 0);
            else
                low[2] = TILE(24, 0, 0);
        }
    }
}

DRAW_FUNC(ground_draw) {
    vu16 *low = GET_LOW(level, xt, yt);

    low[0] = TILE(8, 0, 0);
    low[1] = TILE(8, 0, 0);
    low[32] = TILE(8, 0, 0);
    low[33] = TILE(8, 0, 0);

    draw_outer_borders(level, xt, yt);
}

const struct tile_Type tile_type_list[TILE_TYPES] = {
    [TILE_VOID] = {
        .is_solid = false,

        .draw = NULL
    },

    [TILE_GROUND] = {
        .is_solid = false,

        .draw = ground_draw
    },
    [TILE_HIGH_GROUND] = {
        .is_solid = true
    },

    [TILE_PLATFORM] = {
        .is_solid = false
    },
    [TILE_FALL_PLATFORM] = {
        .is_solid = false
    },
    [TILE_HOLE] = {
        .is_solid = false
    },

    [TILE_WOOD] = {
        .is_solid = true
    },
    [TILE_ROCK] = {
        .is_solid = true
    },
    [TILE_WATER] = {
        .is_solid = true
    }
};
