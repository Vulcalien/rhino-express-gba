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
        low[64] = TILE(12, 0, 0);
        low[65] = TILE(12, 0, 0);

        if(left)
            low[63] = TILE(13, 1, 0);
        if(right)
            low[66] = TILE(13, 0, 0);
    }

    if(left) {
        low[31] = TILE(down ? 7 : 19, 1, 0);

        if(up)
            low[-1] = TILE(1, 1, 0);
        else
            low[-1] = TILE(top_left ? 19 : 18, 1, 0);
    }

    if(right) {
        low[34] = TILE(down ? 7 : 19, 0, 0);

        if(up)
            low[2] = TILE(1, 0, 0);
        else
            low[2] = TILE(top_right ? 19 : 18, 0, 0);
    }
}

static INLINE bool editor_on_top(struct Level *level, i32 xt, i32 yt) {
    return level->is_editing      &&
           level->editor.xt == xt &&
           level->editor.yt == yt;
}

DRAW_FUNC(ground_draw) {
    vu16 *low = GET_LOW(level, xt, yt);

    low[0]  = TILE(6, 0, 0);
    low[1]  = TILE(6, 0, 0);
    low[32] = TILE(6, 0, 0);
    low[33] = TILE(6, 0, 0);

    draw_outer_borders(level, xt, yt);
}

DRAW_FUNC(high_ground_draw) {
    ground_draw(level, xt, yt);

    vu16 *high = GET_HIGH(level, xt, yt);

    bool left  = level_get_tile(level, xt - 1, yt) != TILE_HIGH_GROUND;
    bool right = level_get_tile(level, xt + 1, yt) != TILE_HIGH_GROUND;
    bool up    = level_get_tile(level, xt, yt - 1) != TILE_HIGH_GROUND;
    bool down  = level_get_tile(level, xt, yt + 1) != TILE_HIGH_GROUND;

    high[0]  = TILE((left  && up) ? 2 : 8, 1, 0);
    high[1]  = TILE((right && up) ? 2 : 8, 0, 0);
    high[32] = TILE((left  && down) ? 14 : 8, 1, 0);
    high[33] = TILE((right && down) ? 14 : 8, 0, 0);

    if(down) {
        high[64] = TILE(left  ? 20 : 12, 1, 0);
        high[65] = TILE(right ? 20 : 12, 0, 0);
    }
}

DRAW_FUNC(platform_draw) {
    vu16 *low = GET_LOW(level, xt, yt);

    u32 tile    = 3;
    u32 palette = 0;
    if(editor_on_top(level, xt, yt)) {
        // green color
        tile    = 9;
        palette = 1;

        // check if there is a solid entity (player or mailbox) on tile
        for(u32 i = 0; i < LEVEL_SOLID_ENTITIES_IN_TILE; i++) {
            const u32 t = xt + yt * LEVEL_W;
            const level_EntityID id = level->solid_entities[t][i];

            if(id < LEVEL_ENTITY_LIMIT) {
                // red color
                tile    = 10;
                palette = 0;
                break;
            }
        }
    }

    low[0]  = TILE(tile, 0, palette);
    low[1]  = TILE(tile, 1, palette);
    low[32] = TILE(tile, 2, palette);
    low[33] = TILE(tile, 3, palette);

    draw_outer_borders(level, xt, yt);
}

DRAW_FUNC(fall_platform_draw) {
    vu16 *low = GET_LOW(level, xt, yt);

    u32 tile    = 4;
    u32 palette = 1;
    if(editor_on_top(level, xt, yt)) {
        tile    = 10;
        palette = 0;
    }

    low[0]  = TILE(tile, 0, palette);
    low[1]  = TILE(tile, 1, palette);
    low[32] = TILE(tile, 2, palette);
    low[33] = TILE(tile, 3, palette);

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
                                 u32 base, u32 palette) {
    vu16 *low = GET_LOW(level, xt, yt);

    u8 data = level_get_data(level, xt, yt);
    bool platform = data & BIT(0);
    bool flip     = data & BIT(1);

    if(platform)
        base += 12 + 12 * editor_on_top(level, xt, yt);

    low[0]  = TILE(base     + flip, flip, palette);
    low[1]  = TILE(base + 1 - flip, flip, palette);
    low[32] = TILE(base + 6 + flip, flip, palette);
    low[33] = TILE(base + 7 - flip, flip, palette);

    draw_outer_borders(level, xt, yt);
}

DRAW_FUNC(wood_draw) {
    draw_obstacle(level, xt, yt, 24, 1);
}

DRAW_FUNC(rock_draw) {
    draw_obstacle(level, xt, yt, 26, 0);
}

DRAW_FUNC(water_draw) {
    draw_obstacle(level, xt, yt, 28, 0);
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
