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
#include <memory.h>

#include "screen.h"
#include "music.h"

#define PAGE_COUNT 9

// number of sprites used for text
#define TEXT_SPRITES 6

#define FADING_NONE  0
#define FADING_IMAGE 1
#define FADING_TEXT  2

static u8 image_in_page[PAGE_COUNT + 1] = {
    0, 0,
    1, 1, 1,
    2, 2, 2, 2,
    255 // array terminator
};

static u32 page;

static struct {
    u8 element; // one of the FADING constants

    i8  dir;
    u16 val;
} transparency;

static void start_init(void *data) {
    page = 0;

    // DEBUG
    transparency.element = FADING_IMAGE;
    transparency.dir = +1;
    transparency.val = 0;

    screen_mode_4();

    // clear the display with the backdrop color
    memset32((vu32 *) display_get_raster(0), 0, 240 * 160);

    // play music
    MUSIC_PLAY(music_map);
}

static inline void update_fading(void) {
    switch(transparency.element) {
        case FADING_IMAGE:
            transparency.val += transparency.dir;

            if(transparency.val == 0) {
                // make the image fade-in
                transparency.dir = +1;
                page++;
            } else if(transparency.val == 16) {
                // make the text fade-in
                transparency.element = FADING_TEXT;
                transparency.dir = +1;
                transparency.val = 0;
            }
            break;

        case FADING_TEXT:
            transparency.val += transparency.dir;

            if(transparency.val == 0) {
                // check if the image should also be changed (note that
                // image_in_page has a terminator at PAGE_COUNT + 1)
                if(image_in_page[page] != image_in_page[page + 1]) {
                    // make the image fade-out
                    transparency.element = FADING_IMAGE;
                    transparency.dir = -1;
                    transparency.val = 16;
                } else {
                    // make the text fade-in
                    transparency.dir = +1;
                    page++;
                }
            } else if(transparency.val == 16) {
                transparency.element = FADING_NONE;
            }
            break;
    }
}

static void start_tick(void) {
    if(tick_count % 2 == 0)
        update_fading();

    // if all pages have been shown, transition to the map scene
    if(page == PAGE_COUNT)
        scene_transition_to(&scene_map, NULL);

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
    if(page == PAGE_COUNT) {
        sprite_hide_all();

        return;
    }

    const u32 image = image_in_page[page];
    const u32 text = page;

    // TODO do not repeat this every draw cycle
    dma_config(DMA3, &(struct DMA) { .chunk = DMA_CHUNK_32_BIT });
    dma_transfer(
        DMA3,
        display_charblock(5) + 64 * 64,
        cutscenes + (64 * 64) * image,
        64 * 64 / 4
    );

    dma_transfer(
        DMA3,
        display_charblock(5) + 96 * 64,
        cutscenes_text + (24 * 64) * text,
        24 * 64 / 4
    );

    const u32 image_x0 = (240 - 64) / 2;
    const u32 image_y0 = (160 - 64) / 2 - 32;

    const u32 text_x0 = (240 - 32 * TEXT_SPRITES) / 2;
    const u32 text_y0 = (160 - 8) / 2 + 48;

    const u32 first_sprite = SCREEN_FOG_PARTICLE_COUNT;

    // draw image sprite
    sprite_config(first_sprite, &(struct Sprite) {
        .x = image_x0,
        .y = image_y0,

        // set mode to semi-transparent or normal
        .mode = (transparency.element == FADING_IMAGE ? 1 : 0),

        .size = SPRITE_SIZE_64x64,

        .tile = 256 + 128,
        .colors = 1
    });

    // draw text sprites
    for(u32 i = 0; i < TEXT_SPRITES; i++) {
        sprite_config(first_sprite + 1 + i, &(struct Sprite) {
            .x = text_x0 + 32 * i,
            .y = text_y0,

            // if the image is being faded, do not show text sprites
            .disable = (transparency.element == FADING_IMAGE ? 1 : 0),

            // set mode to semi-transparent or normal
            .mode = (transparency.element == FADING_TEXT ? 1 : 0),

            .size = SPRITE_SIZE_32x8,

            .tile = 256 + 192 + i * 4,
            .colors = 1
        });
    }

    // set color effects (only apply to semi-transparent sprites)
    display_blend(
        &(struct DisplayTarget) { 0 },
        &(struct DisplayTarget) { .backdrop = 1 },
        transparency.val, 16 - transparency.val
    );
}

const struct Scene scene_start = {
    .init = start_init,
    .tick = start_tick,
    .draw = start_draw
};
