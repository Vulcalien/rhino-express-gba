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

#define MUSIC_PLAY(sound) do {           \
    audio_play(7, sound, sizeof(sound)); \
    audio_loop(7, sizeof(sound));        \
    audio_pitch(7, 0x1000);              \
} while(0)

extern const u8 music_game[260624];
extern const u8 music_map[130312];
extern const u8 music_editing[130312];
