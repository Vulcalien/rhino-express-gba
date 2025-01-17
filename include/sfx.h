/* Copyright 2024-2025 Vulcalien
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

#include <gba/audio.h>

extern u32 _sfx_playing_priority;

INLINE void _sfx_play(const i8 *sound, u32 length, u32 priority) {
    // This is a hack that works only if these assumptions are true:
    // - the audio driver only has two channels
    // - channel 1 is always busy (i.e. playing music)

    if(priority >= _sfx_playing_priority) {
        audio_play(0, sound, length);
        _sfx_playing_priority = priority;
    } else {
        // try to play only if the channel is free
        bool could_play = (audio_play(-1, sound, length) == 0);

        // If the previous, higher priority sound had already stopped,
        // then '_sfx_playing_priority' should be updated.
        if(could_play)
            _sfx_playing_priority = priority;
    }
}

#define SFX_PLAY(sound, priority) \
    _sfx_play((i8 *) sound, sizeof(sound), priority)

extern const u8 sfx_delivery[4686];
extern const u8 sfx_player_step[1004];
extern const u8 sfx_player_spawn[8144];
extern const u8 sfx_player_fall[8096];
extern const u8 sfx_player_win[11264];
extern const u8 sfx_falling_platform[9420];
extern const u8 sfx_obstacle_placed[1906];
extern const u8 sfx_obstacle_hit[887];
extern const u8 sfx_obstacle_broken[3944];
extern const u8 sfx_water[8492];
