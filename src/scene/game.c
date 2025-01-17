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
#include <gba/background.h>
#include <gba/input.h>
#include <memory.h>

#include "level.h"
#include "screen.h"

static struct Level level;

static inline void setup_tutorial_text(void) {
    // clear first tile of tileset
    memory_clear_32(display_charblock(1), 32);

    // clear visible tiles of tilemap
    memory_clear_32(BG1_TILEMAP, 32 * 20 * 2);

    // set tiles of tilemap
    for(u32 y = 0; y < 3; y++)
        for(u32 x = 0; x < 16; x++)
            BG1_TILEMAP[(7 + x) + (15 + y) * 32] = 1 + x + y * 16;
}

static void game_init(u32 selected_level) {
    level.attempts = 0;
    level_load(&level, &level_metadata[selected_level]);

    background_toggle(BG2, true); // level's higher tiles
    background_toggle(BG3, true); // level's lower tiles

    setup_tutorial_text();

    // draw now to prevent showing garbage on the first frame
    scene_game.draw();
}

static void game_tick(void) {
    // if START is pressed, transition to map without clearing level
    if(input_pressed(KEY_START))
        scene_transition_to(&scene_map, BIT(1));

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
