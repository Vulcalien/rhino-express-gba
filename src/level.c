/* Copyright 2023-2024 Vulcalien
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
#include "level.h"

#include <gba/display.h>
#include <gba/background.h>
#include <gba/sprite.h>
#include <memory.h>
#include <random.h>

#include "screen.h"
#include "entity.h"
#include "tile.h"
#include "music.h"

#include "res/img/tutorial-text.c"
#include "res/img/level-sidebar.c"

static inline void insert_solid_entity(struct Level *level,
                                       struct entity_Data *data,
                                       level_EntityID id,
                                       i32 xt, i32 yt) {
    if(xt < 0 || yt < 0 || xt >= LEVEL_W || yt >= LEVEL_H)
        return;

    const u32 tile = xt + yt * LEVEL_W;
    for(u32 i = 0; i < LEVEL_SOLID_ENTITIES_IN_TILE; i++) {
        if(level->solid_entities[tile][i] >= LEVEL_ENTITY_LIMIT) {
            level->solid_entities[tile][i] = id;
            data->solid_id = i;

            break;
        }
    }
}

static inline void remove_solid_entity(struct Level *level,
                                       struct entity_Data *data,
                                       level_EntityID id,
                                       i32 xt, i32 yt) {
    if(xt < 0 || yt < 0 || xt >= LEVEL_W || yt >= LEVEL_H)
        return;

    const u32 tile = xt + yt * LEVEL_W;
    if(level->solid_entities[tile][data->solid_id] == id)
        level->solid_entities[tile][data->solid_id] = LEVEL_NO_ENTITY;
}

static inline void update_offset(struct Level *level) {
    const struct level_Metadata *metadata = level->metadata;

    const u32 width_pixels  = metadata->size.w << LEVEL_TILE_SIZE;
    const u32 height_pixels = metadata->size.h << LEVEL_TILE_SIZE;

    level->offset.x = -(DISPLAY_W - width_pixels) / 2;
    level->offset.y = -(DISPLAY_H - height_pixels) / 2;

    // apply shaking effect
    if(level->shake_time > 0) {
        level->shake_time--;

        level->offset.y += (-2 + random(4));

        // shake tutorial text
        background_mosaic(random(2), random(2));
    } else {
        // disable shaking of tutorial text
        background_mosaic(0, 0);
    }
}

static inline void tick_entities(struct Level *level) {
    for(u32 i = 0; i < LEVEL_ENTITY_LIMIT; i++) {
        struct entity_Data *data = &level->entities[i];
        if(!entity_is_valid(data))
            continue;

        i32 xt0 = data->x >> LEVEL_TILE_SIZE;
        i32 yt0 = data->y >> LEVEL_TILE_SIZE;

        const struct entity_Type *entity_type = entity_get_type(data);
        entity_type->tick(level, data);

        if(data->should_remove) {
            if(entity_type->is_solid)
                remove_solid_entity(level, data, i, xt0, yt0);

            data->type = ENTITY_INVALID;
        } else if(entity_type->is_solid) {
            i32 xt1 = data->x >> LEVEL_TILE_SIZE;
            i32 yt1 = data->y >> LEVEL_TILE_SIZE;

            if(xt1 != xt0 || yt1 != yt0) {
                remove_solid_entity(level, data, i, xt0, yt0);
                insert_solid_entity(level, data, i, xt1, yt1);
            }
        }
    }
}

IWRAM_SECTION
void level_tick(struct Level *level) {
    update_offset(level);

    tick_entities(level);

    // update shaking status
    if(level->shake) {
        level->shake_time = 15;
        level->shake = false;
    }
}

static inline void draw_tiles(struct Level *level) {
    const struct level_Metadata *metadata = level->metadata;

    background_offset(BG2, level->offset.x, level->offset.y + 5);
    background_offset(BG3, level->offset.x, level->offset.y);

    for(u32 y = 0; y < metadata->size.h; y++) {
        for(u32 x = 0; x < metadata->size.w; x++) {
            const struct tile_Type *type = tile_get_type(
                level_get_tile(level, x, y)
            );
            if(!type || !type->draw)
                continue;

            type->draw(level, x, y);
        }
    }
}

static inline void draw_entities(struct Level *level) {
    u32 used_sprites = SCREEN_FOG_PARTICLE_COUNT;
    for(level_EntityID id = 0; id < LEVEL_ENTITY_LIMIT; id++) {
        struct entity_Data *data = &level->entities[id];
        const struct entity_Type *type = entity_get_type(data);
        if(!type)
            continue;

        const i32 draw_x = data->x - level->offset.x;
        const i32 draw_y = data->y - level->offset.y;

        used_sprites += type->draw(
            level, data, draw_x, draw_y, used_sprites
        );
        if(used_sprites >= SPRITE_COUNT)
            break;
    }
    sprite_hide_range(used_sprites, SPRITE_COUNT);
}

IWRAM_SECTION
void level_draw(struct Level *level) {
    // clear the tilemap before redrawing
    memory_clear_32(BG2_TILEMAP, 32 * 32 * 2);
    memory_clear_32(BG3_TILEMAP, 32 * 32 * 2);

    // toggle tutorial text's background
    background_toggle(BG1, level->metadata->tutorial);

    draw_tiles(level);
    draw_entities(level);
}

static inline void level_init(struct Level *level,
                              const struct level_Metadata *metadata) {
    level->metadata = metadata;

    // clear 'data'
    for(u32 i = 0; i < LEVEL_SIZE; i++)
        level->data[i] = 0;

    // clear 'entities'
    for(u32 i = 0; i < LEVEL_ENTITY_LIMIT; i++)
        level->entities[i].type = ENTITY_INVALID;

    // clear 'solid_entities'
    for(u32 t = 0; t < LEVEL_SIZE; t++)
        for(u32 i = 0; i < LEVEL_SOLID_ENTITIES_IN_TILE; i++)
            level->solid_entities[t][i] = LEVEL_NO_ENTITY;
}


static inline void load_tiles(struct Level *level) {
    const struct level_Metadata *metadata = level->metadata;

    const u8 *tiles = metadata->tile_data;
    for(u32 y = 0; y < metadata->size.h; y++) {
        for(u32 x = 0; x < metadata->size.w; x++) {
            u8 tile = tiles[x + y * metadata->size.w];
            u8 data = 0;

            // translate obstacles-with-platform pseudo-tiles and set
            // the platform bit
            if(tile >= 12 && tile <= 14) {
                tile -= 6;
                data |= BIT(0);
            }

            // if the tile is an obstacle, randomly set the flip bit
            if(tile >= TILE_WOOD && tile <= TILE_WATER)
                if(random(2) == 0)
                    data |= BIT(1);

            level_set_tile(level, x, y, tile);
            level_set_data(level, x, y, data);
        }
    }
}

static inline void load_mailboxes(struct Level *level) {
    const struct level_Metadata *metadata = level->metadata;

    level->letters_to_deliver = 0;
    while(true) {
        u32 x = metadata->mailboxes[level->letters_to_deliver].x;
        u32 y = metadata->mailboxes[level->letters_to_deliver].y;

        // the list is terminated by (0, 0)
        if(x == 0 && y == 0)
            break;

        level_add_mailbox(level, x, y);

        level->letters_to_deliver++;
    }
}

static inline void load_decor_houses(struct Level *level) {
    const struct level_Metadata *metadata = level->metadata;

    for(u32 i = 0; true; i++) {
        u32 x = metadata->houses[i].x;
        u32 y = metadata->houses[i].y;

        bool lower = metadata->houses[i].lower;

        // the list is terminated by (0, 0)
        if(x == 0 && y == 0)
            break;

        level_add_decor_house(level, x, y, lower);
    }
}

IWRAM_SECTION
void level_load(struct Level *level,
                const struct level_Metadata *metadata) {
    level_init(level, metadata);
    update_offset(level);

    load_tiles(level);
    load_mailboxes(level);
    load_decor_houses(level);

    // load tutorial text
    if(metadata->tutorial) {
        memory_copy_32(
            (vu8 *) display_charblock(1) + 1 * 32,
            (vu8 *) tutorial_text + metadata->tutorial_text * 48 * 32,
            48 * 32
        );
    }

    // load edit sidebar
    memory_copy_32(
        (vu8 *) display_charblock(4) + 128 * 32,
        level_sidebar,
        4 * 8 * 32
    );

    // copy 'obstacles' array
    for(u32 i = 0; i < 3; i++)
        level->editor.obstacles[i] = metadata->obstacles[i];

    // enter editing mode
    level->editor.active = true;
    level_add_edit_sidebar(level);
    level_add_edit_cursor(level);
    level->editor.xt = metadata->size.w / 2;
    level->editor.yt = metadata->size.h / 2;
    MUSIC_PLAY(music_editing);

    // add player
    level_add_player(level);
}

IWRAM_SECTION
level_EntityID level_new_entity(struct Level *level) {
    for(u32 i = 0; i < LEVEL_ENTITY_LIMIT; i++) {
        struct entity_Data *data = &level->entities[i];
        if(!entity_is_valid(data)) {
            memory_clear(data->data, ENTITY_EXTRA_DATA_SIZE);
            return i;
        }
    }
    return LEVEL_NO_ENTITY;
}

void level_add_entity(struct Level *level,
                      enum entity_TypeID type,
                      level_EntityID id) {
    if(id >= LEVEL_ENTITY_LIMIT)
        return;

    struct entity_Data *data = &level->entities[id];
    data->type = type;
    data->should_remove = false;

    const struct entity_Type *entity_type = entity_get_type(data);
    if(entity_type && entity_type->is_solid) {
        i32 xt = data->x >> LEVEL_TILE_SIZE;
        i32 yt = data->y >> LEVEL_TILE_SIZE;

        insert_solid_entity(level, data, id, xt, yt);
    }
}

// Levels

#include "res/levels/1.c"
#include "res/levels/2.c"
#include "res/levels/3.c"
#include "res/levels/4.c"
#include "res/levels/5.c"
#include "res/levels/6.c"
#include "res/levels/7.c"
#include "res/levels/8.c"
#include "res/levels/9.c"
#include "res/levels/10.c"
#include "res/levels/11.c"
#include "res/levels/12.c"
#include "res/levels/13.c"
#include "res/levels/14.c"
#include "res/levels/15.c"
#include "res/levels/16.c"
#include "res/levels/17.c"

const struct level_Metadata level_metadata[LEVEL_COUNT] = {
    // Level 1
    {
        .tile_data = level_1,
        .size = { 7, 3 },
        .spawn = { 1, 1 },

        .mailboxes = {
            { 4, 1 }
        },
        .houses = {
            { 5, 1 }
        },

        .tutorial = true,
        .tutorial_text = 0
    },

    // Level 2
    {
        .tile_data = level_2,
        .size = { 7, 8 },
        .spawn = { 5, 3 },

        .mailboxes = {
            { 3, 2 },
            { 3, 5 }
        },
        .houses = {
            { 1, 6 }
        }
    },

    // Level 3
    {
        .tile_data = level_3,
        .size = { 6, 8 },
        .spawn = { 1, 4 },

        .mailboxes = {
            { 2, 2 },
            { 3, 4 }
        },
        .houses = {
            { 1, 1 },
            { 4, 3 }
        }
    },

    // Level 4
    {
        .tile_data = level_4,
        .size = { 7, 5 },
        .spawn = { 5, 1 },

        .mailboxes = {
            { 2, 2 }
        },
        .houses = {
            { 1, 1, true }
        }
    },

    // Level 5
    {
        .tile_data = level_5,
        .size = { 7, 8 },
        .spawn = { 5, 4 },

        .mailboxes = {
            { 3, 2 },
            { 2, 4 }
        },
        .houses = {
            { 1, 1 },
            { 4, 2 },
            { 4, 6 }
        }
    },

    // Level 6
    {
        .tile_data = level_6,
        .size = { 10, 7 },
        .spawn = { 4, 3 },

        .mailboxes = {
            { 5, 2 },
            { 7, 3 },
            { 3, 4 }
        },
        .houses = {
            { 6, 1 },
            { 8, 3 },
            { 1, 4 }
        }
    },

    // Level 7
    {
        .tile_data = level_7,
        .size = { 6, 3 },
        .spawn = { 1, 1 },
        .obstacles = { 1, 0, 0 },

        .mailboxes = {
            { 3, 1 }
        },

        .tutorial = true,
        .tutorial_text = 1
    },

    // Level 8
    {
        .tile_data = level_8,
        .size = { 6, 6 },
        .spawn = { 2, 4 },
        .obstacles = { 2, 0, 0 },

        .mailboxes = {
            { 2, 2 }
        },
        .houses = {
            { 1, 1 }
        }
    },

    // Level 9
    {
        .tile_data = level_9,
        .size = { 8, 7 },
        .spawn = { 2, 5 },
        .obstacles = { 0, 2, 0 },

        .mailboxes = {
            { 2, 2 },
            { 5, 5 }
        },
        .houses = {
            { 1, 3 },
            { 1, 4, true }
        }
    },

    // Level 10
    {
        .tile_data = level_10,
        .size = { 11, 6 },
        .spawn = { 5, 4 },
        .obstacles = { 0, 1, 0 },

        .mailboxes = {
            { 2, 2 },
            { 8, 3 }
        },
        .houses = {
            { 1, 1 },
            { 9, 2, true }
        }
    },

    // Level 11
    {
        .tile_data = level_11,
        .size = { 7, 7 },
        .spawn = { 2, 3 },
        .obstacles = { 0, 3, 0 },

        .mailboxes = {
            { 3, 2 },
            { 4, 3 },
            { 2, 4 }
        },
        .houses = {
            { 3, 1 },
            { 1, 2 },
            { 1, 5 }
        }
    },

    // Level 12
    {
        .tile_data = level_12,
        .size = { 10, 4 },
        .spawn = { 4, 2 },

        .mailboxes = {
            { 2, 2 }
        },
        .houses = {
            { 1, 1 }
        },

        .tutorial = true,
        .tutorial_text = 2
    },

    // Level 13
    {
        // TODO this is a tutorial level
        .tile_data = level_13,
        .size = { 6, 11 },
        .spawn = { 1, 6 },

        .mailboxes = {
            { 3, 2 }
        },
        .houses = {
            { 2, 1 }
        }
    },

    // Level 14
    {
        .tile_data = level_14,
        .size = { 8, 7 },
        .spawn = { 1, 5 },
        .obstacles = { 1, 1, 0 },

        .mailboxes = {
            { 5, 2 }
        },
        .houses = {
            { 4, 1 }
        },

        .tutorial = true,
        .tutorial_text = 3
    },

    // Level 15
    {
        .tile_data = level_15,
        .size = { 9, 5 },
        .spawn = { 4, 3 },
        .obstacles = { 1, 0, 1 },

        .mailboxes = {
            { 2, 3 },
            { 7, 2 }
        },
        .houses = {
            { 1, 1 },
            { 1, 2 }
        }
    },

    // Level 16
    {
        .tile_data = level_16,
        .size = { 9, 9 },
        .spawn = { 4, 4 },
        .obstacles = { 2, 1, 1 },

        .mailboxes = {
            { 3, 2 },
            { 2, 3 },
            { 3, 6 },
            { 6, 6 }
        },
        .houses = {
            { 1, 1 },
            { 2, 1 },
            { 1, 2 },
            { 1, 3 }
        }
    },

    // Level 17
    {
        .tile_data = level_17,
        .size = { 11, 7 },
        .spawn = { 1, 1 },
        .obstacles = { 3, 1, 2 },

        .mailboxes = {
            { 5, 1 },
            { 7, 2 },
            { 3, 3 },
            { 8, 3 },
            { 4, 4 }
        },
        .houses = {
            { 1, 4 },
            { 1, 5 }
        }
    }
};
