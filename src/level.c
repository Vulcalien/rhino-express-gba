/* Copyright 2023-2025 Vulcalien
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
#include "editor.h"
#include "music.h"

#include "res/img/tutorial-text.c"

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

    const i32 width_pixels  = metadata->size.w << LEVEL_TILE_SIZE;
    const i32 height_pixels = metadata->size.h << LEVEL_TILE_SIZE;

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

    bool was_editing = level->editing;
    editor_tick(level);
    if(was_editing && !level->editing)
        MUSIC_PLAY(music_game);

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

static inline void draw_entities(struct Level *level, u32 *used_sprites) {
    for(level_EntityID id = 0; id < LEVEL_ENTITY_LIMIT; id++) {
        struct entity_Data *data = &level->entities[id];
        const struct entity_Type *type = entity_get_type(data);
        if(!type)
            continue;

        const i32 draw_x = data->x - level->offset.x;
        const i32 draw_y = data->y - level->offset.y;

        // check if entity is out of display bounds
        if(draw_x < -32 || draw_x >= DISPLAY_W + 32 ||
           draw_y < -32 || draw_y >= DISPLAY_H + 32)
            continue;

        *used_sprites += type->draw(
            level, data, draw_x, draw_y, *used_sprites
        );
        if(*used_sprites >= SPRITE_COUNT)
            break;
    }
}

IWRAM_SECTION
void level_draw(struct Level *level) {
    // clear the tilemap before redrawing
    memory_clear_32(BG2_TILEMAP, 32 * 32 * 2);
    memory_clear_32(BG3_TILEMAP, 32 * 32 * 2);

    // toggle tutorial text's background
    background_toggle(BG1, level->metadata->tutorial);

    draw_tiles(level);

    u32 used_sprites = SCREEN_FOG_PARTICLE_COUNT;
    editor_draw(level, &used_sprites);
    draw_entities(level, &used_sprites);
    sprite_hide_range(used_sprites, SPRITE_COUNT);
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

            if(metadata->tutorial_bubbles) {
                if(tile == TILE_WOOD || tile == TILE_ROCK)
                    level_add_particle_tutorial_bubble(level, x, y, tile);
            }

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

static inline void load_decorations(struct Level *level) {
    const struct level_Metadata *metadata = level->metadata;

    // add houses
    for(u32 i = 0; true; i++) {
        const u32 x = metadata->houses[i].x;
        const u32 y = metadata->houses[i].y;

        bool lower = metadata->houses[i].lower;

        // the list is terminated by (0, 0)
        if(x == 0 && y == 0)
            break;

        level_add_decor_house(level, x, y, lower);
    }

    // add grass
    for(u32 i = 0; true; i++) {
        const u32 x = metadata->grass[i].x;
        const u32 y = metadata->grass[i].y;

        // the list is terminated by (0, 0)
        if(x == 0 && y == 0)
            break;

        level_add_decor_grass(level, x, y);
    }
}

IWRAM_SECTION
void level_load(struct Level *level,
                const struct level_Metadata *metadata) {
    // use a fixed pseudo-random seed, so that every time the level is
    // reloaded (e.g. after the player falls) the random details do not
    // change.
    u32 old_seed = random_seed(256);

    level_init(level, metadata);
    update_offset(level);

    load_tiles(level);
    load_mailboxes(level);
    load_decorations(level);

    // load tutorial text
    if(metadata->tutorial) {
        memory_copy_32(
            (vu8 *) display_charblock(1) + 1 * 32,
            (vu8 *) tutorial_text + metadata->tutorial_text * 48 * 32,
            48 * 32
        );
    }

    // add player
    level_add_player(level);

    // initialize editor
    editor_init(level);

    if(!level->editing)
        MUSIC_PLAY(music_game);

    // restore pseudo-random seed
    random_seed(old_seed);
}

IWRAM_SECTION
level_EntityID level_new_entity(struct Level *level) {
    for(u32 i = 0; i < LEVEL_ENTITY_LIMIT; i++) {
        struct entity_Data *data = &level->entities[i];
        if(!entity_is_valid(data)) {
            memory_clear(data->extra, ENTITY_EXTRA_SIZE);
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
