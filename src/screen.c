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

#define LOAD_TILESET(dest, tileset)\
    memcpy16((dest), (vu16 *) (tileset), sizeof(tileset))

#define LOAD_PALETTE(dest, palette)\
    memcpy16((dest), (vu16 *) (palette), sizeof(palette))

void screen_init(void) {
    display_config(&(struct Display) {
        .mode = 0,

        .oam_hblank  = 0,
        .obj_mapping = 1,

        .enable_bg0 = 0,
        .enable_bg1 = 0,
        .enable_bg2 = 1,
        .enable_bg3 = 1,
        .enable_obj = 1
    });

    // configure backgrounds
    for(u32 i = 0; i < BACKGROUND_COUNT; i++)
        background_config(i, &bg_configs[i]);

    // load tileset
    LOAD_TILESET(background_get_tileset(BG2), tileset);
    LOAD_TILESET(OBJ_TILESET, sprites);

    // load palette
    LOAD_PALETTE(DISPLAY_BG_PALETTE,  palette);
    LOAD_PALETTE(DISPLAY_OBJ_PALETTE, palette);

    // hide all sprites
    for(u32 i = 0; i < SPRITE_COUNT; i++)
        sprite_hide(i);

    // disable forced blank
    display_force_blank(false);
}
