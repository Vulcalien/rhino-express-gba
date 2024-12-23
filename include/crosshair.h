/* Copyright 2024 Vulcalien
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
#pragma once

#include "main.h"

#include <gba/sprite.h>

// A mini-module for drawing a crosshair sprite

// returns the sprites it drew
INLINE u32 crosshair_draw(u32 id, i32 xc, i32 yc) {
    const u32 distance = ((tick_count / 32) & 1) ? 1 : 3;
    const u32 tile = 88;

    sprite_config(id++, &(struct Sprite) {
        .x = xc - distance - 8,
        .y = yc - distance - 8,

        .tile = tile,
        .palette = 0
    });

    sprite_config(id++, &(struct Sprite) {
        .x = xc + distance,
        .y = yc - distance - 8,

        .flip = 1,

        .tile = tile,
        .palette = 0
    });

    sprite_config(id++, &(struct Sprite) {
        .x = xc - distance - 8,
        .y = yc + distance,

        .flip = 2,

        .tile = tile,
        .palette = 0
    });

    sprite_config(id++, &(struct Sprite) {
        .x = xc + distance,
        .y = yc + distance,

        .flip = 3,

        .tile = tile,
        .palette = 0
    });

    return 4;
}
