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

        particles[i].x = 256 * random(DISPLAY_W);
        particles[i].y = 256 * random(DISPLAY_H);

        particles[i].xm = random(17) - 8;
        particles[i].ym = random(17) - 8;
    }
}

#define LOAD_TILESET(dest, tileset)\
    memory_copy_32((dest), (tileset), sizeof(tileset))

#define LOAD_PALETTE(dest, palette)\
    memory_copy_32((dest), (palette), sizeof(palette))

void screen_init(void) {
    display_config(0);
    sprite_hide(-1);

    // tutorial text and map
    background_config(BG1, &(struct Background) {
        .priority = 1,
        .tileset  = 1,
        .tilemap  = 2,

        .mosaic = 1
    });

    // level's higher tiles
    background_config(BG2, &(struct Background) {
        .priority = 2,
        .tileset  = 3,
        .tilemap  = 4
    });

    // level's lower tiles
    background_config(BG3, &(struct Background) {
        .priority = 3,
        .tileset  = 3,
        .tilemap  = 6
    });

    // load tileset and spritesheet
    LOAD_TILESET(display_charblock(3), tileset);
    LOAD_TILESET(display_charblock(4), sprites);

    // load palette
    LOAD_PALETTE(DISPLAY_BG_PALETTE,  palette);
    LOAD_PALETTE(DISPLAY_OBJ_PALETTE, palette);

    init_fog_particles();

    // disable forced blank
    display_force_blank(false);
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

            .size = SPRITE_SIZE_8x8,

            .tile = 56 + particles[i].tile,
            .palette = 1
        });
    }
}
