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

#include "level.h"
#include "sprite.h"
#include "input.h"
#include "math.h"

struct player_Data {
    i8 xm;
    i8 ym;

    u8 sprite_flip;

    u8 unused[13];
};

static_assert(
    sizeof(struct player_Data) == ENTITY_EXTRA_DATA_SIZE,
    "struct player_Data is of wrong size"
);

IWRAM_SECTION
static void player_tick(struct Level *level, struct entity_Data *data) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    // TODO ...
}

static inline u32 calculate_inverse_y_scale(u32 arg) {
    arg *= MATH_PI / 32;
    u32 val = math_max(math_sin(arg), 0) * 43 / 0x4000;
    return 256 - val;
}

IWRAM_SECTION
static inline draw_letters(struct Level *level, i32 x, i32 y,
                           u32 used_sprites) {
    struct Sprite sprite = {
        .tile = 52,
        .color_mode = 1
    };

    // TODO ...
    return 0;
}

IWRAM_SECTION
static u32 player_draw(struct Level *level, struct entity_Data *data,
                       u32 used_sprites) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    // 8-bit fixed point decimal (1 = 256)
    u32 inverse_y_scale = calculate_inverse_y_scale(tick_count);

    struct Sprite sprite = {
        .x = data->x - 16,
        .y = data->y - (256 * 16) / inverse_y_scale,

        .size = 1,
        .flip = player_data->sprite_flip,

        .tile = 0,
        .color_mode = 1,

        .affine_transformation = 1,
        .affine_parameter = 0,
        .double_size = 1
    };
    sprite_set(&sprite, used_sprites++);

    // set OBJ parameter
    vu16 *parameters = &OAM[3];
    parameters[0]  = 256;
    parameters[4]  = 0;
    parameters[8]  = 0;
    parameters[12] = inverse_y_scale;

    return 1 + draw_letters(level, data->x, data->y, used_sprites);
}

const struct entity_Type entity_player = {
    .xr = 8,
    .yr = 8,

    .is_solid = true,

    .tick = player_tick,
    .draw = player_draw
};

bool level_add_player(struct Level *level, u32 xt, u32 yt) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    // set generic entity data
    struct entity_Data *data = &level->entities[id];

    data->x = (xt << LEVEL_TILE_SIZE) + 8;
    data->y = (yt << LEVEL_TILE_SIZE) + 8;

    // set specific player data
    struct player_Data *player_data = (struct player_Data *) &data->data;

    player_data->xm = 0;
    player_data->ym = 0;

    player_data->sprite_flip = 0; // TODO is this right?

    level_add_entity(level, ENTITY_PLAYER, id);
    return true;
}
