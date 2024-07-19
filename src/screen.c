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
#include <random.h>
#include <math.h>

#include "res/img/tileset.c"
#include "res/img/sprites.c"
#include "res/img/palette.c"

static struct {
    u8 tile;

    i32 x;
    i32 y;

    i16 xm;
    i16 ym;
} particles[SCREEN_FOG_PARTICLE_COUNT];

static inline void init_fog_particles(void) {
    for(u32 i = 0; i < SCREEN_FOG_PARTICLE_COUNT; i++) {
        particles[i].tile = 0;

        particles[i].x = 256 * random(SCREEN_W);
        particles[i].y = 256 * random(SCREEN_H);

        particles[i].xm = random(17) - 8;
        particles[i].ym = random(17) - 8;
    }
}

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
    LOAD_TILESET(display_get_charblock(5), sprites);

    // load palette
    LOAD_PALETTE(DISPLAY_BG_PALETTE,  palette);
    LOAD_PALETTE(DISPLAY_OBJ_PALETTE, palette);

    // disable forced blank
    display_force_blank(false);

    init_fog_particles();
}

IWRAM_SECTION
void screen_draw_fog_particles(u32 first_sprite_id) {
    for(u32 i = 0; i < SCREEN_FOG_PARTICLE_COUNT; i++) {
        // update particle tile
        if(random(4) == 0) {
            if(random(2) == 0 && particles[i].tile > 0)
                particles[i].tile--;
            else if(particles[i].tile < 7)
                particles[i].tile++;
        }

        // randomly change velocity
        particles[i].xm += random(9) - 4;
        particles[i].ym += random(9) - 4;

        if(math_abs(particles[i].xm) > 128)
            particles[i].xm /= 2;
        if(math_abs(particles[i].ym) > 128)
            particles[i].ym /= 2;

        // add velocity to particle position
        particles[i].x += particles[i].xm;
        particles[i].y += particles[i].ym;

        // draw sprite
        sprite_config(first_sprite_id + i, &(struct Sprite) {
            .x = (particles[i].x / 256) % 256 - 4,
            .y = (particles[i].y / 256) % 256 - 4,

            .tile = 512 + 112 + 2 * particles[i].tile,
            .colors = 1
        });
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
