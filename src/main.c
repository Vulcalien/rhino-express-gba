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
#include "main.h"

#include <gba/interrupt.h>
#include <gba/audio.h>
#include <gba/input.h>

#include "screen.h"
#include "performance.h"
#include "scene.h"
#include "storage.h"

u32 tick_count = 0;
u32 levels_cleared = 0;

static inline void tick(void) {
    input_update();
    scene->tick();

    performance_tick();

    tick_count++;
}

static inline void draw(void) {
    scene->draw();
    screen_draw_fog_particles(0);

    performance_draw();
}

IWRAM_SECTION
static void vblank(void) {
    performance_vblank();
}

int AgbMain(void) {
    // initialize drivers
    interrupt_init();
    audio_init();

    interrupt_toggle(IRQ_VBLANK, true);
    interrupt_set_isr(IRQ_VBLANK, vblank);

    screen_init();
    scene_set(&scene_start, 0);

    storage_load();

    while(true) {
        tick();

        vsync();
        draw();
    }
    return 0;
}
