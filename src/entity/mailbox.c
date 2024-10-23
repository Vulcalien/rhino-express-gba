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

#include <gba/sprite.h>

#include "level.h"

struct mailbox_Data {
    u16 animation;

    bool has_letter;

    u8 unused[13];
};

static_assert(
    sizeof(struct mailbox_Data) == ENTITY_EXTRA_DATA_SIZE,
    "struct mailbox_Data is of wrong size"
);

IWRAM_SECTION
static void mailbox_tick(struct Level *level, struct entity_Data *data) {
    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->data;

    // TODO
}

IWRAM_SECTION
static u32 mailbox_draw(struct Level *level, struct entity_Data *data,
                        i32 x, i32 y, u32 used_sprites) {
    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->data;

    sprite_config(used_sprites++, &(struct Sprite) {
        // TODO better adjust the coordinates to match the original
        .x = x - 16,
        .y = y - 16,

        .size = SPRITE_SIZE_16x16,

        .tile = 512 + 8 + mailbox_data->has_letter * 8,
        .colors = 1,

        .affine = 1,
        .affine_parameter = 1, // TODO
        .double_size = 1
    });

    // DEBUG
    vu16 *parameters = &OAM[16 + 3];
    parameters[0]  = 256;
    parameters[4]  = 0;
    parameters[8]  = 0;
    parameters[12] = 256;

    return 1;
}

IWRAM_SECTION
static bool mailbox_touched_by(struct Level *level,
                               struct entity_Data *data,
                               struct entity_Data *touching_data) {
    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->data;

    if(touching_data->type != ENTITY_PLAYER || mailbox_data->has_letter)
        return false;

    mailbox_data->has_letter = true;
    mailbox_data->animation = 0; // TODO animation

    level->letters_to_deliver--;

    return false;
}

const struct entity_Type entity_mailbox = {
    .xr = 8,
    .yr = 8,

    .is_solid = true,

    .tick = mailbox_tick,
    .draw = mailbox_draw,

    .touched_by = mailbox_touched_by
};

bool level_add_mailbox(struct Level *level, u32 xt, u32 yt) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    // set generic entity data
    struct entity_Data *data = &level->entities[id];

    data->x = (xt << LEVEL_TILE_SIZE) + 8;
    data->y = (yt << LEVEL_TILE_SIZE) + 8;

    // set specific mailbox data
    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->data;

    mailbox_data->animation = 0;
    mailbox_data->has_letter = false;

    level_add_entity(level, ENTITY_MAILBOX, id);
    return true;
}
