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

#include "background.h"
#include "sprite.h"
#include "random.h"

#include "entity.h"
#include "tile.h"
#include "music.h"

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

void level_init(struct Level *level) {
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

    // enter editing mode
    level->is_editing = true;
    SOUND_PLAY(music_editing, true, SOUND_CHANNEL_B);
}

static inline void tick_tiles(struct Level *level) {
    // ...
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
    tick_tiles(level);
    tick_entities(level);
}

static inline void draw_tiles(struct Level *level) {
    background_set_offset(BG2, level->offset.x, level->offset.y + 5);
    background_set_offset(BG3, level->offset.x, level->offset.y);

    for(u32 y = 0; y < LEVEL_H; y++) {
        for(u32 x = 0; x < LEVEL_W; x++) {
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
    u32 used_sprites = 0;
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

    for(u32 i = used_sprites; i < SPRITE_COUNT; i++)
        sprite_hide(i);
}

IWRAM_SECTION
void level_draw(struct Level *level) {
    draw_tiles(level);
    draw_entities(level);
}

IWRAM_SECTION
void level_load(struct Level *level, const u8 tiles[LEVEL_SIZE]) {
    for(u32 y = 0; y < LEVEL_H; y++) {
        for(u32 x = 0; x < LEVEL_W; x++) {
            const u8 tile = tiles[x + y * LEVEL_W];
            if(tile >= TILE_TYPES)
                continue;

            level_set_tile(level, x, y, tile);

            u8 data = 0;
            switch(tile) {
                case TILE_WOOD:
                case TILE_ROCK:
                case TILE_WATER:
                    data = rand() & 1;
                    break;
            }
            level_set_data(level, x, y, data);
        }
    }
}

IWRAM_SECTION
level_EntityID level_new_entity(struct Level *level) {
    for(u32 i = 0; i < LEVEL_ENTITY_LIMIT; i++) {
        struct entity_Data *data = &level->entities[i];
        if(!entity_is_valid(data))
            return i;
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
