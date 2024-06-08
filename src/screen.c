/* Copyright 2023-2024 Vulcalien
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
#include "screen.h"

#include "background.h"
#include "sprite.h"
#include "palette.h"
#include "util.h"

#define DISPLAY_CONTROL *((vu16 *) 0x04000000)
#define DISPLAY_STATUS  *((vu16 *) 0x04000004)

#define WINDOW_IN  *((vu16 *) 0x04000048)
#define WINDOW_OUT *((vu16 *) 0x0400004a)

#define CHAR_BLOCK_0 ((vu16 *) 0x06000000)
#define CHAR_BLOCK_1 ((vu16 *) 0x06004000)
#define CHAR_BLOCK_2 ((vu16 *) 0x06008000)
#define CHAR_BLOCK_3 ((vu16 *) 0x0600c000)

#define OBJ_TILESET ((vu16 *) 0x06010000)

static const struct Background bg_configs[BACKGROUND_COUNT] = {
    // BG0
    {
        .priority = 0,
        .tileset  = 0,
        .tilemap  = 0,

        .colors = 1
    },

    // BG1
    {
        .priority = 1,
        .tileset  = 0,
        .tilemap  = 2,

        .colors = 1
    },

    // BG2
    {
        .priority = 2,
        .tileset  = 3,
        .tilemap  = 4,

        .colors = 1
    },

    // BG3
    {
        .priority = 3,
        .tileset  = 3,
        .tilemap  = 6,

        .colors = 1
    }
};

#include "res/tileset.c"
#include "res/sprites.c"
#include "res/palette.c"

void screen_init(void) {
    DISPLAY_CONTROL = 0 << 0  | // Video mode
                      1 << 6  | // OBJ character mapping (1 = linear)
                      1 << 7  | // Forced blank
                      0 << 8  | // Enable BG 0
                      0 << 9  | // Enable BG 1
                      1 << 10 | // Enable BG 2
                      1 << 11 | // Enable BG 3
                      1 << 12;  // Enable OBJs

    // configure backgrounds
    for(u32 i = 0; i < BACKGROUND_COUNT; i++)
        background_config(i, &bg_configs[i]);

    // load tileset
    memcpy16(CHAR_BLOCK_3, (vu16 *) tileset, sizeof(tileset));
    memcpy16(OBJ_TILESET,  (vu16 *) sprites, sizeof(tileset));

    // load palette
    memcpy16(PALETTE_BG,  palette, sizeof(palette));
    memcpy16(PALETTE_OBJ, palette, sizeof(palette));

    // hide all sprites
    for(u32 i = 0; i < SPRITE_COUNT; i++)
        sprite_hide(i);

    // disable forced blank
    DISPLAY_CONTROL &= ~(1 << 7);
}
