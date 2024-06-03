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

    level_add_edit_cursor(&level, 8, 5);
    level_add_player(&level, 8, 7);
    level_add_mailbox(&level, 8, 7);

    level.obstacles_to_add[0] = 1;
    level.obstacles_to_add[1] = 1;
    level.obstacles_to_add[2] = 1;

    level_load(&level, &level_metadata[0]);
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
