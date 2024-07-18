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
#include <gba/input.h>
#include <gba/dma.h>

#include "screen.h"

// number of sprites used for text
#define TEXT_SPRITES 6

#define FADING_NONE  0
#define FADING_IMAGE 1
#define FADING_TEXT  2

static u32 page;

static struct {
    u8 element; // one of the FADING constants

    i8  dir;
    u16 val;
} transparency;

static void start_init(void *data) {
    page = 0;

    // DEBUG
    transparency.element = FADING_TEXT;
    transparency.dir = -1;
    transparency.val = 16;

    screen_mode_4();
}

static inline void update_transparency(void) {
    switch(transparency.element) {
        case FADING_NONE:
            // TODO
            break;

        case FADING_IMAGE:
            transparency.val += transparency.dir;

            if(transparency.val == 16) {
                transparency.element = FADING_TEXT;
                transparency.val = 0;
            } else if(transparency.val == 0) {
                transparency.dir = +1;
            }
            break;

        case FADING_TEXT:
            transparency.val += transparency.dir;

            if(transparency.val == 0) {
                transparency.element = FADING_IMAGE;
                transparency.dir = -1;
                transparency.val = 16;
            } else if(transparency.val == 16) {
                transparency.element = FADING_NONE;
            }
            break;
    }
}

static void start_tick(void) {
    if(tick_count % 2 == 0)
        update_transparency();

    if(transparency.element == FADING_NONE) {
        if(input_pressed(KEY_A) || input_pressed(KEY_B) ||
           input_pressed(KEY_START)) {
            transparency.element = FADING_TEXT;
            transparency.dir = -1;
            transparency.val = 16;
        }
    }
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

    const u32 image_x0 = (240 - 64) / 2;
    const u32 image_y0 = (160 - 64) / 2 - 32;

    const u32 text_x0 = (240 - 32 * TEXT_SPRITES) / 2;
    const u32 text_y0 = (160 - 8) / 2 + 48;

    // draw image sprite
    sprite_config(0, &(struct Sprite) {
        .x = image_x0,
        .y = image_y0,

        .size = 3, // 64x64

        .tile = 512 + 256, // TODO
        .colors = 1
    });

    // draw text sprites
    for(u32 i = 0; i < TEXT_SPRITES; i++) {
        sprite_config(1 + i, &(struct Sprite) {
            .x = text_x0 + 32 * i,
            .y = text_y0,

            .shape = 1, // horizontal
            .size = 1, // 32x8

            .tile = 512 + 384 + i * 8,
            .colors = 1
        });
    }

    // set color effects
    display_blend(
        &(struct DisplayTarget) { .obj = 1 },
        &(struct DisplayTarget) { .backdrop = 1 },
        transparency.val, 16 - transparency.val
    );

    // set windows
    display_window_enable(DISPLAY_WINDOW_0);
    display_window_enable(DISPLAY_WINDOW_1);

    display_window_config(DISPLAY_WINDOW_OUT, &(struct DisplayWindow) {
        .obj = 1, .effects = 0
    });

    display_window_viewport(
        DISPLAY_WINDOW_0, image_x0, image_y0, 64, 64
    );
    display_window_viewport(
        DISPLAY_WINDOW_1, text_x0, text_y0, 32 * TEXT_SPRITES, 8
    );

    switch(transparency.element) {
        case FADING_NONE:
            display_window_disable(DISPLAY_WINDOW_0);
            display_window_disable(DISPLAY_WINDOW_1);
            break;
        case FADING_IMAGE:
            display_window_config(
                DISPLAY_WINDOW_0,
                &(struct DisplayWindow) { .obj = 1, .effects = 1 }
            );
            display_window_config(
                DISPLAY_WINDOW_1,
                &(struct DisplayWindow) { .obj = 0, .effects = 0 }
            );
            break;
        case FADING_TEXT:
            display_window_config(
                DISPLAY_WINDOW_0,
                &(struct DisplayWindow) { .obj = 1, .effects = 0 }
            );
            display_window_config(
                DISPLAY_WINDOW_1,
                &(struct DisplayWindow) { .obj = 1, .effects = 1 }
            );
            break;
    }
}

const struct Scene scene_start = {
    .init = start_init,
    .tick = start_tick,
    .draw = start_draw
};
