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
#pragma once

#include "main.h"

#define BG0_TILEMAP display_screenblock(0)
#define BG1_TILEMAP display_screenblock(2)
#define BG2_TILEMAP display_screenblock(4)
#define BG3_TILEMAP display_screenblock(6)

#define SCREEN_FOG_PARTICLE_COUNT 10

extern void screen_init(void);

extern void screen_draw_fog_particles(u32 first_sprite_id);
