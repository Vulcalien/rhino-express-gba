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

#include "level.h"

static struct Level level;

static void game_init(void *data) {
    u32 selected_level = *((u32 *) data);

    level_load(&level, &level_metadata[selected_level]);
}

static void game_tick(void) {
    level_tick(&level);
}

static void game_draw(void) {
    level_draw(&level);
}

const struct Scene scene_game = {
    .init = game_init,
    .tick = game_tick,
    .draw = game_draw
};
