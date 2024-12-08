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

#define SFX_PLAY(sound) sound_play(0, sound, sizeof(sound))

extern const u8 sfx_delivery[5429];
extern const u8 sfx_player_spawn[8144];
extern const u8 sfx_player_fall[8096];
extern const u8 sfx_player_win[11264];
extern const u8 sfx_falling_platform[9420];
extern const u8 sfx_obstacle_placed[1906];
extern const u8 sfx_obstacle_hit[887];
extern const u8 sfx_obstacle_broken[3944];
extern const u8 sfx_water[9107];
