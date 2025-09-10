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

THUMB
static void start_init(u32 data) {
    page = 0;

    // DEBUG
    transparency.element = FADING_IMAGE;
    transparency.dir = +1;
    transparency.val = 0;

    // clear the display with the backdrop color
    memory_clear_32(display_get_raster(0), DISPLAY_WIDTH * DISPLAY_HEIGHT);

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

THUMB
static void start_tick(void) {
    if(tick_count % 2 == 0)
        update_fading();

    // if all pages have been shown, transition to the map scene
    if(page == PAGE_COUNT)
        scene_transition_to(&scene_map, BIT(2));

    if(transparency.element == FADING_NONE) {
        if(input_press(KEY_A) || input_press(KEY_B) ||
           input_press(KEY_START)) {
            transparency.element = FADING_TEXT;
            transparency.dir = -1;
            transparency.val = 16;
        }
    }
}

#include "../res/img/cutscenes.c"
#include "../res/img/cutscenes-text.c"

THUMB
static void start_draw(void) {
    // hide all sprites, so that they are not shown when transitioning
    sprite_hide(-1);

    if(page == PAGE_COUNT)
        return;

    const u32 image = image_in_page[page];
    const u32 text = page;

    // TODO do not repeat this every draw cycle
    dma_config(DMA3, &(struct DMA) { .chunk = DMA_CHUNK_32_BIT });
    dma_transfer(
        DMA3,
        (vu8 *) display_charblock(4) + 128 * 32,
        cutscenes + (64 * 32) * image,
        64 * 32 / 4
    );

    dma_transfer(
        DMA3,
        (vu8 *) display_charblock(4) + 192 * 32,
        cutscenes_text + (24 * 32) * text,
        24 * 32 / 4
    );

    const u32 image_x0 = (DISPLAY_WIDTH - 64) / 2;
    const u32 image_y0 = (DISPLAY_HEIGHT - 64) / 2 - 32;

    const u32 text_x0 = (DISPLAY_WIDTH - 32 * TEXT_SPRITES) / 2;
    const u32 text_y0 = (DISPLAY_HEIGHT - 8) / 2 + 48;

    const u32 first_sprite = SCREEN_FOG_PARTICLE_COUNT;

    // draw image sprite
    sprite_config(first_sprite, &(struct Sprite) {
        .x = image_x0,
        .y = image_y0,

        // set mode to semi-transparent or normal
        .mode = (transparency.element == FADING_IMAGE ? 1 : 0),

        .size = SPRITE_SIZE_64x64,

        .tile = 128,
        .palette = (image == 1 ? 0 : 2)
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

            .tile = 192 + i * 4,
            .palette = 0
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
