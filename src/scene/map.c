/* Copyright 2024-2025 Vulcalien
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
#include <gba/dma.h>
#include <memory.h>
#include <random.h>
#include <math.h>

#include "level.h"
#include "screen.h"
#include "crosshair.h"
#include "storage.h"
#include "music.h"

#include "../res/img/map.c"
#include "../res/img/map-paths.c"
#include "../res/img/level-buttons.c"

#define PAGE_COUNT 4
static u8 first_level_in_pages[PAGE_COUNT] = {
    0, 6, 11, 17
};

static u16 draw_offset;
static u32 grass_seed; // random seed used to draw grass

static i8 page;
static i8 level;

static bool block_movement;

THUMB
static void map_init(u32 data) {
    bool has_cleared_level = (data & BIT(0));
    bool play_music        = (data & BIT(1));
    bool select_next_level = (data & BIT(2));

    if(has_cleared_level && level == levels_cleared) {
        levels_cleared++;
        storage_save();
    }

    if(play_music)
        MUSIC_PLAY(music_map);

    if(select_next_level) {
        level = levels_cleared;

        // select page
        if(levels_cleared < LEVEL_COUNT) {
            level = levels_cleared;
            for(u32 i = 0; i < PAGE_COUNT - 1; i++) {
                if(first_level_in_pages[i + 1] > level) {
                    page = i;
                    break;
                }
            }
        } else {
            level = 0;
            page = 0;
        }

        draw_offset = page * 240;
    }

    // randomly choose a seed for drawing grass
    grass_seed = random(0x10000) | random(0x10000) << 16;

    // unblock movement between level buttons
    block_movement = false;

    // set tilemap tiles
    for(u32 y = 0; y < 20; y++)
        for(u32 x = 0; x < 31; x++)
            BG1_TILEMAP[x + y * 32] = (x + y * 31) | 1 << 12;

    // load paths tileset
    memory_copy_32(display_charblock(5), map_paths, sizeof(map_paths));

    // draw now to prevent showing garbage on the first frame
    scene_map.draw();
}

static inline void update_draw_offset(void) {
    if(draw_offset != page * 240) {
        u32 diff = page * 240 - draw_offset;

        u32 speed;
        if(math_abs(diff) > 140)
            speed = 12;
        else if(math_abs(diff) > 100)
            speed = 10;
        else if(math_abs(diff) > 60)
            speed = 8;
        else if(math_abs(diff) > 20)
            speed = 6;
        else if(math_abs(diff) > 4)
            speed = 4;
        else
            speed = 2;

        draw_offset += speed * math_sign(diff);
    }
}

THUMB
static void map_tick(void) {
    if(!block_movement) {
        // move left/right by one page
        if(input_pressed(KEY_L)) {
            page--;

            if(page < 0) {
                page = 0;
                level = 0;
            } else {
                level = first_level_in_pages[page + 1] - 1;
            }
        }
        if(input_pressed(KEY_R)) {
            page++;

            if(page > PAGE_COUNT - 1) {
                page = PAGE_COUNT - 1;
                level = levels_cleared;
            } else {
                level = first_level_in_pages[page];
            }
        }

        // move left/right by one level
        level -= (input_pressed(KEY_LEFT) || input_pressed(KEY_UP));
        level += (input_pressed(KEY_RIGHT) || input_pressed(KEY_DOWN));
    }

    if(level < 0)
        level = 0;
    if(level > levels_cleared)
        level = levels_cleared;

    // adjust the selected page
    page -= (level < first_level_in_pages[page]);
    page += (level >= first_level_in_pages[page + 1]);

    update_draw_offset();

    // check if the player has chosen a level
    if(input_pressed(KEY_A) || input_pressed(KEY_START)) {
        if(level < LEVEL_COUNT) {
            scene_transition_to(&scene_game, level);
            block_movement = true;
        }
    }
}

static inline void draw_level_buttons(u32 *used_sprites) {
    const struct {
        // coordinates of center
        i16 x;
        i16 y;
    } level_buttons[LEVEL_COUNT] = {
        { 76,  40  },
        { 76,  80  },
        { 92,  124 },
        { 132, 104 },
        { 144, 56  },
        { 176, 88  },

        { 316, 38  },
        { 324, 90  },
        { 364, 126 },
        { 392, 86  },
        { 396, 42  },

        { 548, 37  },
        { 580, 61  },
        { 556, 93  },
        { 588, 129 },
        { 624, 97  },
        { 632, 53  }
    };

    const u32 buttons = math_min(levels_cleared + 1, LEVEL_COUNT);
    for(u32 i = 0; i < buttons; i++) {
        // calculate the button's top-left corner
        const i32 x = level_buttons[i].x - draw_offset - 8;
        const i32 y = level_buttons[i].y - 8;

        if(x < -16 || x >= 240)
            continue;

        // load button image
        u32 image = i * 4;
        image += 68  * (i == level);          // selected level
        image += 136 * (i == levels_cleared); // uncleared level

        memory_copy_32(
            (vu8 *) display_charblock(4) + (128 + i * 4) * 32,
            level_button_images + image * 32,
            4 * 32
        );

        if(i == level) {
            // draw crosshair
            crosshair_draw(*used_sprites, x + 8, y + 8);
            *used_sprites += 4;
        }

        sprite_config((*used_sprites)++, &(struct Sprite) {
            .x = x,
            .y = y,

            .size = SPRITE_SIZE_16x16,

            .tile = 128 + i * 4,
            .palette = 2
        });
    }
}

static inline void draw_paths(u32 *used_sprites) {
    const struct {
        u8 level;

        // coordinates of top-left corner
        i16 x;
        i16 y;
    } paths[] = {
        { 1, 71, 48 },

        { 2, 73, 88 },

        { 3, 100, 107 },
        { 3, 116, 107 },

        { 4, 133, 62 },
        { 4, 133, 94 },

        { 5, 149, 55 },
        { 5, 165, 55 },

        { 6, 183, 66 },
        { 6, 199, 66 },
        { 6, 215, 50 },
        { 6, 231, 50 },
        { 6, 247, 34 },
        { 6, 263, 34 },
        { 6, 279, 34 },
        { 6, 295, 34 },

        { 7, 315, 46 },
        { 7, 315, 78 },

        { 8, 329, 94 },
        { 8, 345, 94 },

        { 9, 366, 92 },
        { 9, 382, 92 },

        { 10, 389, 49 },

        { 11, 404, 33 },
        { 11, 420, 33 },
        { 11, 436, 33 },
        { 11, 452, 33 },
        { 11, 468, 33 },
        { 11, 484, 33 },
        { 11, 500, 33 },
        { 11, 516, 33 },
        { 11, 532, 33 },

        { 12, 550, 41 },
        { 12, 566, 41 },

        { 13, 557, 69 },
        { 13, 573, 69 },

        { 14, 558, 99 },
        { 14, 574, 99 },

        { 15, 595, 100 },
        { 15, 611, 100 },

        { 16, 623, 60 },

        { 17, 640, 50 },
        { 17, 656, 50 },
        { 17, 672, 50 },
        { 17, 688, 50 },
        { 17, 704, 50 },
        { 17, 720, 50 },
        { 17, 736, 50 },
        { 17, 752, 50 },
        { 17, 768, 50 },
        { 17, 784, 50 },
        { 17, 800, 50 },
        { 17, 816, 50 }
    };

    for(u32 i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        if(paths[i].level > levels_cleared)
            break;

        const i32 x = paths[i].x - draw_offset;
        const i32 y = paths[i].y;

        // check if sprite would be inside display area
        if(x < -16 || x >= 240)
            continue;

        sprite_config((*used_sprites)++, &(struct Sprite) {
            .x = x,
            .y = y,

            .size = SPRITE_SIZE_16x32,

            .tile = 512 + i * 8,
            .palette = 0
        });
    }
}

static inline void draw_grass(u32 *used_sprites) {
    const u32 old_random_seed = random_seed(grass_seed);

    const struct {
        // coordinates of top-left corner
        i16 x;
        i16 y;
    } grass[] = {
        { 84,  24  },
        { 52,  44  },
        { 160, 44  },
        { 116, 60  },
        { 96,  72  },
        { 144, 72  },
        { 96,  96  },
        { 108, 96  },
        { 72,  120 },

        { 296, 18  },
        { 400, 20  },
        { 380, 22  },
        { 380, 58  },
        { 332, 62  },
        { 308, 98  },
        { 380, 114 },

        { 576, 13  },
        { 644, 25  },
        { 544, 57  },
        { 640, 61  },
        { 552, 65  },
        { 588, 105 },
        { 600, 129 },

        { 828, 52 },
        { 828, 68 },
        { 848, 72 },
        { 824, 80 }
    };

    for(u32 i = 0; i < sizeof(grass) / sizeof(grass[0]); i++) {
        const i32 x = grass[i].x - draw_offset;
        const i32 y = grass[i].y;

        // check if sprite would be inside display area
        if(x < -16 || x >= 240)
            continue;

        sprite_config((*used_sprites)++, &(struct Sprite) {
            .x = x,
            .y = y,

            .size = SPRITE_SIZE_8x8,

            .tile = 48 + random(4),
            .palette = 1
        });
    }

    // restore random seed
    random_seed(old_random_seed);
}

static inline void draw_page_arrows(u32 *used_sprites) {
    // fixed point number: 1 = 0x4000
    // scale = 1.25 + sin(t) / 4   --->   range [1, 1.5]
    i32 scale = 0x5000 + math_sin(tick_count * math_brad(90) / 16) / 4;

    // left arrows
    sprite_affine(0, (i16 [4]) {
        256 * 0x4000 / scale, 0,
        0, 256 * 0x4000 / scale
    });

    // right arrows
    sprite_affine(1, (i16 [4]) {
        -256 * 0x4000 / scale, 0,
        0, 256 * 0x4000 / scale
    });

    for(u32 i = 1; i < PAGE_COUNT * 2 - 1; i++) {
        const i32 y = 24;
        const i32 x = (i / 2) * 240 + 120
                      + (i % 2 ? +1 : -1) * 96
                      - draw_offset;

        if(x < -16 || x >= 240 + 16)
            continue;

        // if next page is not unlocked, do not draw right arrow
        if(i % 2 == 1)
            if(levels_cleared < first_level_in_pages[i / 2 + 1])
                break;

        sprite_config((*used_sprites)++, &(struct Sprite) {
            .x = x - 16,
            .y = y - 16,

            .size = SPRITE_SIZE_16x16,

            .tile = 28,
            .palette = 2,

            .affine = 1,
            .affine_parameter = i % 2,
            .double_size = 1
        });
    }
}

THUMB
static void map_draw(void) {
    background_toggle(BG1, true);  // map
    background_toggle(BG2, false); // level's higher tiles
    background_toggle(BG3, false); // level's lower tiles

    // draw sprites
    u32 used_sprites = SCREEN_FOG_PARTICLE_COUNT;

    draw_page_arrows(&used_sprites);
    draw_level_buttons(&used_sprites);
    draw_paths(&used_sprites);
    draw_grass(&used_sprites);

    sprite_hide_range(used_sprites, SPRITE_COUNT);

    // draw tilemap
    u32 x0 = draw_offset / 8;
    for(u32 y = 0; y < 20; y++) {
        // select the y-th tilemap row
        vu8 *dest = (vu8 *) display_charblock(1) + (y * 31) * 32;

        // select the left-most tile (rows are 120 tiles long)
        vu8 *src = (vu8 *) map + (y * 120 + x0) * 32;

        dma_config(DMA3, &(struct DMA) { .chunk = DMA_CHUNK_32_BIT });
        dma_transfer(DMA3, dest, src, 31 * 32 / 4);
    }
    background_offset(BG1, draw_offset % 8, 0);
}

const struct Scene scene_map = {
    .init = map_init,
    .tick = map_tick,
    .draw = map_draw
};
