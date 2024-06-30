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

#define OBJ_TILESET ((vu16 *) 0x06014000)

#include "res/tileset.c"
#include "res/sprites.c"
#include "res/palette.c"

#define LOAD_TILESET(dest, tileset)\
    memcpy16((dest), (vu16 *) (tileset), sizeof(tileset))

#define LOAD_PALETTE(dest, palette)\
    memcpy16((dest), (vu16 *) (palette), sizeof(palette))

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
    LOAD_TILESET(OBJ_TILESET, sprites);

    // load palette
    LOAD_PALETTE(DISPLAY_BG_PALETTE,  palette);
    LOAD_PALETTE(DISPLAY_OBJ_PALETTE, palette);

    // disable forced blank
    display_force_blank(false);
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

    LOAD_TILESET(background_get_tileset(BG2), tileset);

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
