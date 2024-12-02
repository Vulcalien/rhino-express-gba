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

#include <gba/sound.h>

#define SFX_PLAY(sound) SOUND_DMA_PLAY(sound, false, SOUND_DMA_A)

extern const u8 sfx_delivery[5429];
extern const u8 sfx_obstacle_placed[1906];
extern const u8 sfx_obstacle_hit[887];
extern const u8 sfx_water[9107];
