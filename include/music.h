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

#define MUSIC_PLAY(sound) do {           \
    sound_play(1, sound, sizeof(sound)); \
    sound_loop(1, sizeof(sound));        \
} while(0)

extern const u8 music_game[260624];
extern const u8 music_map[130312];
extern const u8 music_editing[130312];
