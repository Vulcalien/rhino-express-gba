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
#include <random.h>
#include <math.h>

#include "level.h"
#include "sfx.h"

#define ANIMATION_TIME 14

struct mailbox_Data {
    u16 animation;
    bool flip;

    bool has_letter;

    u8 unused[12];
};
ASSERT_SIZE(struct mailbox_Data, ENTITY_EXTRA_SIZE);

IWRAM_SECTION
static void mailbox_tick(struct Level *level, struct entity_Data *data) {
    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->extra;

    if(mailbox_data->animation)
        mailbox_data->animation--;
}

IWRAM_SECTION
static u32 mailbox_draw(struct Level *level, struct entity_Data *data,
                        i32 x, i32 y, u32 used_sprites) {
    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->extra;

    const u32  animation   = mailbox_data->animation;
    const bool flip        = mailbox_data->flip;
    const bool has_letter  = mailbox_data->has_letter;

    const bool should_scale = has_letter && animation > 0;

    // Note: sprites flipped using affine transformations are shifted by
    // one pixel horizontally, so (should_scale && flip) is subtracted.
    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 8 - 8 * should_scale - (should_scale && flip),
        .y = y - 20 - 16 * should_scale,

        .size = should_scale ? SPRITE_SIZE_16x32 : SPRITE_SIZE_16x16,
        .flip = flip,

        .tile = 16 - has_letter * 8,
        .palette = 0,

        .affine = should_scale,
        .affine_parameter = 1,
        .double_size = 1
    });

    if(should_scale) {
        const u32 t = animation * math_brad(180) / ANIMATION_TIME;
        const u32 scale_y = 0x4000 + math_sin(t);

        sprite_affine(1, (i16 [4]) {
            256 * (flip ? -1 : +1), 0,
            0, 256 * 0x4000 / scale_y
        });
    }

    return 1;
}

IWRAM_SECTION
static bool mailbox_touched_by(struct Level *level,
                               struct entity_Data *data,
                               struct entity_Data *touching_data) {
    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->extra;

    if(touching_data->type != ENTITY_PLAYER || mailbox_data->has_letter)
        return false;

    mailbox_data->has_letter = true;
    mailbox_data->animation = ANIMATION_TIME;

    SFX_PLAY(sfx_delivery, 1);
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

    struct entity_Data *data = &level->entities[id];
    data->x = (xt << LEVEL_TILE_SIZE) + 8;
    data->y = (yt << LEVEL_TILE_SIZE) + 8;

    struct mailbox_Data *mailbox_data = (struct mailbox_Data *) &data->extra;
    mailbox_data->animation = 0;
    mailbox_data->flip = random(2);
    mailbox_data->has_letter = false;

    level_add_entity(level, ENTITY_MAILBOX, id);
    return true;
}
