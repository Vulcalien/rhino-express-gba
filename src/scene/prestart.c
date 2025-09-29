/* Copyright 2025 Vulcalien
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

#define FADING_TICKS 64

static i32 time;

THUMB
static void prestart_init(u32 data) {
    time = FADING_TICKS;
}

THUMB
static void prestart_tick(void) {
    time--;
    if(time == 0)
        scene_set(&scene_start, 0);
}

THUMB
static void prestart_draw(void) {
    display_brighten(NULL, time * 32 / FADING_TICKS);
}

const struct Scene scene_prestart = {
    .init = prestart_init,
    .tick = prestart_tick,
    .draw = prestart_draw
};
