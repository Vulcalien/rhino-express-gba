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
#include "entity.h"

IWRAM_SECTION
static void walk_tick(struct Level *level, struct entity_Data *data) {
    // TODO
}

IWRAM_SECTION
static u32 walk_draw(struct Level *level, struct entity_Data *data,
                     u32 used_sprites) {
    // TODO
    return 1;
}

// TODO
const struct entity_Type entity_particle_walk = {
    .xr = 0,
    .yr = 0,

    .is_solid = false,

    .tick = walk_tick,
    .draw = walk_draw
};
