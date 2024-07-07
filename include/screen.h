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

#define SCREEN_W (240)
#define SCREEN_H (160)

#define BG0_TILEMAP ((vu16 *) 0x06000000)
#define BG1_TILEMAP ((vu16 *) 0x06001000)
#define BG2_TILEMAP ((vu16 *) 0x06002000)
#define BG3_TILEMAP ((vu16 *) 0x06003000)

extern void screen_init(void);

extern void screen_mode_0(void);
extern void screen_mode_4(void);
