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

static INLINE u16 get_tile_value(u32 number, u32 flip, u32 palette) {
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

    low[0]  = TILE(8, 0, 0);
    low[1]  = TILE(8, 0, 0);
    low[32] = TILE(8, 0, 0);
    low[33] = TILE(8, 0, 0);

    draw_outer_borders(level, xt, yt);
}

DRAW_FUNC(high_ground_draw) {
    ground_draw(level, xt, yt);

    vu16 *high = GET_HIGH(level, xt, yt);

    bool left  = level_get_tile(level, xt - 1, yt) != TILE_HIGH_GROUND;
    bool right = level_get_tile(level, xt + 1, yt) != TILE_HIGH_GROUND;
    bool up    = level_get_tile(level, xt, yt - 1) != TILE_HIGH_GROUND;
    bool down  = level_get_tile(level, xt, yt + 1) != TILE_HIGH_GROUND;

    high[0]  = TILE((left  && up) ? 2 : 10, 1, 0);
    high[1]  = TILE((right && up) ? 2 : 10, 0, 0);
    high[32] = TILE((left  && down) ? 18 : 10, 1, 0);
    high[33] = TILE((right && down) ? 18 : 10, 0, 0);

    if(down) {
        high[64] = TILE(left  ? 26 : 25, 1, 0);
        high[65] = TILE(right ? 26 : 25, 0, 0);
    }
}

DRAW_FUNC(platform_draw) {
    vu16 *low = GET_LOW(level, xt, yt);

    low[0]  = TILE(3, 0, 0);
    low[1]  = TILE(3, 1, 0);
    low[32] = TILE(3, 2, 0);
    low[33] = TILE(3, 3, 0);

    draw_outer_borders(level, xt, yt);
}

DRAW_FUNC(fall_platform_draw) {
    vu16 *low = GET_LOW(level, xt, yt);

    low[0]  = TILE(4, 0, 0);
    low[1]  = TILE(4, 1, 0);
    low[32] = TILE(4, 2, 0);
    low[33] = TILE(4, 3, 0);

    draw_outer_borders(level, xt, yt);
}

DRAW_FUNC(hole_draw) {
    vu16 *low = GET_LOW(level, xt, yt);

    low[0]  = TILE(5, 0, 0);
    low[1]  = TILE(5, 1, 0);
    low[32] = TILE(5, 2, 0);
    low[33] = TILE(5, 3, 0);

    draw_outer_borders(level, xt, yt);
}

static INLINE void draw_obstacle(struct Level *level, i32 xt, i32 yt,
                                 u32 base) {
    vu16 *low = GET_LOW(level, xt, yt);

    u8 data = level_get_data(level, xt, yt);
    bool platform = data & BIT(0);
    bool flip     = data & BIT(1);

    if(platform)
        base += 16;

    low[0]  = TILE(base     + flip, flip, 0);
    low[1]  = TILE(base + 1 - flip, flip, 0);
    low[32] = TILE(base + 8 + flip, flip, 0);
    low[33] = TILE(base + 9 - flip, flip, 0);

    draw_outer_borders(level, xt, yt);
}

DRAW_FUNC(wood_draw) {
    draw_obstacle(level, xt, yt, 32);
}

DRAW_FUNC(rock_draw) {
    draw_obstacle(level, xt, yt, 34);
}

DRAW_FUNC(water_draw) {
    draw_obstacle(level, xt, yt, 36);
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
        .is_solid = true,

        .draw = high_ground_draw
    },

    [TILE_PLATFORM] = {
        .is_solid = false,

        .draw = platform_draw
    },
    [TILE_FALL_PLATFORM] = {
        .is_solid = false,

        .draw = fall_platform_draw
    },
    [TILE_HOLE] = {
        .is_solid = false,

        .draw = hole_draw
    },

    [TILE_WOOD] = {
        .is_solid = false,

        .draw = wood_draw
    },
    [TILE_ROCK] = {
        .is_solid = false,

        .draw = rock_draw
    },
    [TILE_WATER] = {
        .is_solid = false,

        .draw = water_draw
    }
};
