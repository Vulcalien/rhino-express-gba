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

#include <gba/display.h>
#include <gba/background.h>
#include <gba/sprite.h>
#include <memory.h>

#define WINDOW_IN  *((vu16 *) 0x04000048)
#define WINDOW_OUT *((vu16 *) 0x0400004a)

#include "res/tileset.c"
#include "res/sprites.c"
#include "res/palette.c"

#define LOAD_TILESET(dest, tileset)\
    memcpy16((dest), (vu16 *) (tileset), sizeof(tileset))

void screen_init(void) {
    // configure backgrounds
    background_config(BG2, &(struct Background) {
        .priority = 2,
        .tileset  = 3,
        .tilemap  = 4,

        .colors = 1
    });
    background_config(BG3, &(struct Background) {
        .priority = 3,
        .tileset  = 3,
        .tilemap  = 6,

        .colors = 1
    });

    // load spritesheet
    LOAD_TILESET(display_get_charblock(5), sprites);

    // load palette
    screen_fade(0, 0);

    // disable forced blank
    display_force_blank(false);
}

// val between 0 (min) and 256 (max)
IWRAM_SECTION
void screen_fade(u32 target_color, u32 val) {
    const u32 palette_size = sizeof(palette) / sizeof(u16);

    for(u32 i = 0; i < palette_size; i++) {
        // calculate contributions of target and palette colors
        u32 target_r = (target_color >> 0)  & 0x1f;
        u32 target_g = (target_color >> 5)  & 0x1f;
        u32 target_b = (target_color >> 10) & 0x1f;

        u32 palette_r = (palette[i] >> 0)  & 0x1f;
        u32 palette_g = (palette[i] >> 5)  & 0x1f;
        u32 palette_b = (palette[i] >> 10) & 0x1f;

        u32 color = (
            ((target_r * val + palette_r * (256 - val)) / 256) << 0  |
            ((target_g * val + palette_g * (256 - val)) / 256) << 5  |
            ((target_b * val + palette_b * (256 - val)) / 256) << 10
        );

        DISPLAY_BG_PALETTE[i]  = color;
        DISPLAY_OBJ_PALETTE[i] = color;
    }
}

void screen_mode_0(void) {
    vsync();

    display_config(&(struct Display) {
        .mode = 0,

        .obj_mapping = 1,

        .enable_bg2 = 1,
        .enable_bg3 = 1,
        .enable_obj = 1
    });

    LOAD_TILESET(display_get_charblock(3), tileset);

    sprite_hide_all();
}

void screen_mode_4(void) {
    vsync();

    display_config(&(struct Display) {
        .mode = 4,

        .obj_mapping = 1,

        .enable_bg2 = 1,
        .enable_obj = 1
    });
    display_set_page(0);

    sprite_hide_all();
}
