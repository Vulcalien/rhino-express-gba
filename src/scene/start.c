/* Copyright 2023 Vulcalien
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
#include "scene.h"

#include "level.h"
#include "random.h"

static struct Level level;

static void start_init(void *data) {
    level_init(&level);

    for(u32 y = 3; y < 8; y++)
        for(u32 x = 5; x < 10; x++)
            level_set_tile(&level, x, y, TILE_GROUND);
    level_set_tile(&level, 7, 5, TILE_VOID);
    level_set_tile(&level, 9, 7, TILE_VOID);

    level_set_tile(&level, 8, 7, TILE_PLATFORM);
    level_set_tile(&level, 7, 7, TILE_FALL_PLATFORM);
    level_set_tile(&level, 6, 8, TILE_HOLE);

    level_set_tile(&level, 6, 3, TILE_WOOD);
    level_set_tile(&level, 7, 4, TILE_ROCK);
    level_set_tile(&level, 8, 4, TILE_WATER);

    level_set_tile(&level, 5, 7, TILE_HIGH_GROUND);

    level_set_tile(&level, 6, 4, TILE_HIGH_GROUND);
    level_set_tile(&level, 6, 5, TILE_HIGH_GROUND);
    level_set_tile(&level, 6, 6, TILE_HIGH_GROUND);
    level_set_tile(&level, 7, 2, TILE_HIGH_GROUND);
    level_set_tile(&level, 7, 6, TILE_HIGH_GROUND);

    level_add_player(&level, 1, 7);
}

static void start_tick(void) {
    level_tick(&level);
}

static void start_draw(void) {
    level_draw(&level);
}

const struct Scene scene_start = {
    .init = start_init,
    .tick = start_tick,
    .draw = start_draw
};
