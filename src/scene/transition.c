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
#include <math.h>

#include "screen.h"

#define TRANSITION_TIME 64

static u32 transition_time = 0;
static struct scene_Transition transition_data;

static void transition_init(void *data) {
    // make sure that there are not multiple transitions
    if(transition_time != 0)
        return;

    transition_data = *(struct scene_Transition *) data;
}

static void transition_tick(void) {
    transition_time++;

    if(transition_time < TRANSITION_TIME / 2) {
        transition_data.previous_scene->tick();
    } else if(transition_time == TRANSITION_TIME / 2) {
        if(transition_data.next_scene->init)
            transition_data.next_scene->init(transition_data.init_data);
    } else if(transition_time > TRANSITION_TIME / 2) {
        transition_data.next_scene->tick();
    }

    if(transition_time > TRANSITION_TIME) {
        scene = transition_data.next_scene;

        // reset the transition time to mark its end
        transition_time = 0;
    }
}

static void transition_draw(void) {
    if(transition_time < TRANSITION_TIME / 2)
        transition_data.previous_scene->draw();
    else if(transition_time > TRANSITION_TIME / 2)
        transition_data.next_scene->draw();

    // apply fade effect
    u32 fade = math_sin(
        transition_time * MATH_PI / TRANSITION_TIME
    ) * 20 / 0x4000;
    display_darken(&(struct display_Target) {
        .bg0 = 1, .bg1 = 1, .bg2 = 1, .bg3 = 1,
        .obj = 1, .backdrop = 1
    }, fade);
}

const struct Scene scene_transition = {
    .init = transition_init,
    .tick = transition_tick,
    .draw = transition_draw
};
