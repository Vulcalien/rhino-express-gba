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

#include <gba/display.h>
#include <gba/sprite.h>
#include <gba/dma.h>

#include "screen.h"

static u32 page;

static void start_init(void *data) {
    page = 0;

    screen_mode_4();
}

static void start_tick(void) {
}

#include "../res/img/cutscenes.c"
#include "../res/img/cutscenes-text.c"

IWRAM_SECTION
static void start_draw(void) {
    u32 image = 0; // DEBUG
    u32 text = 0; // DEBUG

    // TODO do not repeat this every draw cycle
    dma_config(DMA3, &(struct DMA) { .chunk = DMA_CHUNK_32_BIT });
    dma_transfer(
        DMA3,
        display_get_charblock(5) + 64 * 64,
        cutscenes + (64 * 64) * image,
        64 * 64 / 4
    );

    dma_transfer(
        DMA3,
        display_get_charblock(5) + 96 * 64,
        cutscenes_text + (24 * 64) * text,
        24 * 64 / 4
    );

    // draw image sprite
    sprite_config(0, &(struct Sprite) {
        .x = (240 - 64) / 2,
        .y = (160 - 64) / 2 - 32,

        .size = 3, // 64x64

        .tile = 512 + 256, // TODO
        .colors = 1
    });

    // draw text sprites
    for(u32 i = 0; i < 6; i++) {
        sprite_config(1 + i, &(struct Sprite) {
            .x = (240 - 32 * 6) / 2 + 32 * i,
            .y = (160 - 8) / 2 + 48,

            .shape = 1, // horizontal
            .size = 1, // 32x8

            .tile = 512 + 384 + i * 8,
            .colors = 1
        });
    }
}

const struct Scene scene_start = {
    .init = start_init,
    .tick = start_tick,
    .draw = start_draw
};
