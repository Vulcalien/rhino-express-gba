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
#include "storage.h"

#include <math.h>
#include <gba/backup.h>

#include "level.h"

bool storage_load(void) {
    if(backup_sram_read(0) != 'Z' ||
       backup_sram_read(1) != 'R' ||
       backup_sram_read(2) != 'E' ||
       backup_sram_read(3) != 'E')
        return false;

    levels_cleared = math_min(backup_sram_read(4), LEVEL_COUNT);

    return true;
}

void storage_save(void) {
    backup_sram_write(0, 'Z');
    backup_sram_write(1, 'R');
    backup_sram_write(2, 'E');
    backup_sram_write(3, 'E');

    backup_sram_write(4, levels_cleared);
}
