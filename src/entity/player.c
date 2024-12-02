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

#include <gba/input.h>
#include <gba/sprite.h>
#include <random.h>
#include <math.h>

#include "level.h"
#include "scene.h"
#include "sfx.h"

#define   MAX_SPEED (1280)
#define START_SPEED (64)

#define BREAK_WOOD_SPEED (546)
#define BREAK_ROCK_SPEED (660)

#define ANIMATION_SPAWN 1
#define ANIMATION_FALL  2
#define ANIMATION_WIN   3

#define FALL_ANIMATION_TIME 60

struct player_Data {
    // subpixel coordinates (128:1)
    i16 subx;
    i16 suby;

    i16 xm;
    i16 ym;

    i8 stored_xm;
    i8 stored_ym;

    u8 animation;
    u8 animation_stage;

    u8 hit_obstacle : 1;
    u8 sprite_flip : 1;

    u8 unused[3];
};
ASSERT_SIZE(struct player_Data, ENTITY_EXTRA_DATA_SIZE);

#define LETTERS_LIMIT (8)

static struct {
    // subpixel coordinates (256:1)
    i32 subx;
    i32 suby;

    u16 angle;
    u16 radius;
} letters[LETTERS_LIMIT];

static inline void set_animation(struct entity_Data *data,
                                 u8 animation) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    player_data->animation = animation;
    player_data->animation_stage = 0;
}

static inline void handle_animation(struct Level *level,
                                    struct entity_Data *data) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    switch(player_data->animation) {
        case ANIMATION_SPAWN: {
            const struct level_Metadata *metadata = level->metadata;
            u32 target_y = (metadata->spawn.y << LEVEL_TILE_SIZE) + 8;

            if(data->y < target_y) {
                data->y += 4;
            } else if(player_data->animation_stage < 15) {
                player_data->animation_stage++;
                level->shake = true;
            } else {
                player_data->animation = 0;
            }
            break;
        }

        case ANIMATION_FALL:
            player_data->animation_stage++;
            if(player_data->animation_stage > FALL_ANIMATION_TIME) {
                // TODO find a way to keep obstacles and other things
                // from changing orientation
                level_load(level, level->metadata);
            }
            break;

        case ANIMATION_WIN:
            player_data->animation_stage++;
            if(player_data->animation_stage > 60 && data->y > -64)
                data->y -= 4;
            if(player_data->animation_stage > 120) {
                static bool map_scene_arg = true;
                scene_transition_to(&scene_map, &map_scene_arg);
            }
            break;

        default:
            player_data->animation = 0;
            break;
    }
}

static inline void read_input(i8 *xm, i8 *ym) {
    if(input_pressed(KEY_UP)) {
        *xm = 0;
        *ym = -1;
    } else if(input_pressed(KEY_LEFT)) {
        *xm = -1;
        *ym = 0;
    } else if(input_pressed(KEY_DOWN)) {
        *xm = 0;
        *ym = +1;
    } else if(input_pressed(KEY_RIGHT)) {
        *xm = +1;
        *ym = 0;
    }
}

static inline void update_sprite_flip(struct entity_Data *data) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    if(player_data->xm < 0)
        player_data->sprite_flip = false;
    else if(player_data->xm > 0)
        player_data->sprite_flip = true;
}

static inline bool is_tile_center(i32 x, i32 y) {
    u32 tile_pixels = 1 << LEVEL_TILE_SIZE;
    return (x % tile_pixels == tile_pixels / 2 &&
            y % tile_pixels == tile_pixels / 2);
}

static inline void enter_tile(struct Level *level,
                              struct entity_Data *data,
                              i32 xt, i32 yt) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    // calculate speed (only one between xm and ym is nonzero)
    const u32 speed = math_abs(player_data->xm + player_data->ym);

    switch(level_get_tile(level, xt, yt)) {
        case TILE_VOID:
        case TILE_HOLE:
            set_animation(data, ANIMATION_FALL);
            break;

        case TILE_WOOD:
            if(speed >= BREAK_WOOD_SPEED) {
                level_set_tile(level, xt, yt, TILE_PLATFORM);
                // TODO play break sound
            } else {
                player_data->hit_obstacle = true;
            }
            level_add_particle_block(level, xt, yt, TILE_WOOD);
            level->shake = true;
            break;

        case TILE_ROCK:
            if(speed >= BREAK_ROCK_SPEED) {
                level_set_tile(level, xt, yt, TILE_PLATFORM);
                // TODO play break sound
            } else {
                player_data->hit_obstacle = true;
            }
            level_add_particle_block(level, xt, yt, TILE_ROCK);
            level->shake = true;
            break;

        case TILE_WATER:
            player_data->xm = player_data->ym = 0;
            player_data->stored_xm = player_data->stored_ym = 0;

            level_add_particle_block(level, xt, yt, TILE_WATER);
            SFX_PLAY(sfx_water);
            break;

        default:
            break;
    }
}

static inline void leave_tile(struct Level *level,
                              struct entity_Data *data,
                              i32 xt, i32 yt) {
    if(level_get_tile(level, xt, yt) == TILE_FALL_PLATFORM) {
        level_set_tile(level, xt, yt, TILE_HOLE);
        level_add_particle_platform(level, xt, yt);
        // TODO add falling platform sound
    }
}

// returns 'true' if the player was able to move exactly by (xm, ym)
// i.e. was not blocked by anything
static inline bool move_full_pixels(struct Level *level,
                                    struct entity_Data *data,
                                    i32 xm, i32 ym) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    while(xm != 0 || ym != 0) {
        bool in_center_before = is_tile_center(data->x, data->y);

        if(!entity_move(level, data, math_sign(xm), math_sign(ym)))
            return false;

        if(in_center_before) {
            // TODO add step particles
        }

        if(is_tile_center(data->x, data->y)) {
            i32 next_xt = data->x >> LEVEL_TILE_SIZE;
            i32 next_yt = data->y >> LEVEL_TILE_SIZE;
            enter_tile(level, data, next_xt, next_yt);

            if(!player_data->hit_obstacle) {
                i32 prev_xt = (data->x >> LEVEL_TILE_SIZE) - math_sign(xm);
                i32 prev_yt = (data->y >> LEVEL_TILE_SIZE) - math_sign(ym);
                leave_tile(level, data, prev_xt, prev_yt);
            }

            // check if movement was canceled
            if(player_data->xm == 0 && player_data->ym == 0)
                break;

            // check if an obstacle was hit
            if(player_data->hit_obstacle) {
                // start moving in the opposite direction
                player_data->xm = -math_sign(player_data->xm);
                player_data->ym = -math_sign(player_data->ym);

                SFX_PLAY(sfx_obstacle_hit);
                break;
            }
        }

        xm -= math_sign(xm);
        ym -= math_sign(ym);
    }
    return true;
}

IWRAM_SECTION
static void player_tick(struct Level *level, struct entity_Data *data) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    if(player_data->animation) {
        handle_animation(level, data);
        return;
    }

    // if the level is still in editing mode, do nothing
    if(level->is_editing)
        return;

    // process player input
    read_input(&player_data->stored_xm, &player_data->stored_ym);

    if(player_data->xm == 0 && player_data->ym == 0) {
        if(level->letters_to_deliver == 0) {
            // all letters are delivered and the player is still
            set_animation(data, ANIMATION_WIN);
            return;
        }

        // set new values for (xm, ym)
        player_data->xm = player_data->stored_xm;
        player_data->ym = player_data->stored_ym;

        // clear (stored_xm, stored_ym)
        player_data->stored_xm = player_data->stored_ym = 0;

        update_sprite_flip(data);
    } else if(player_data->hit_obstacle) {
        // if an obstacle was hit, gradually move back by one tile
        entity_move(
            level, data,
            player_data->xm * 2, player_data->ym * 2
        );

        // if the center was reached, stop going back
        if(is_tile_center(data->x, data->y)) {
            player_data->xm = player_data->ym = 0;
            player_data->hit_obstacle = false;
        }
    } else {
        i32 xm_sign = math_sign(player_data->xm);
        i32 ym_sign = math_sign(player_data->ym);

        // increase speed
        player_data->xm += 2 * xm_sign + player_data->xm / 16;
        player_data->ym += 2 * ym_sign + player_data->ym / 16;

        if(player_data->xm > MAX_SPEED)
            player_data->xm = MAX_SPEED;
        if(player_data->ym > MAX_SPEED)
            player_data->ym = MAX_SPEED;

        // sub-pixel movement
        i32 x_speed = math_max(START_SPEED, math_abs(player_data->xm));
        i32 y_speed = math_max(START_SPEED, math_abs(player_data->ym));

        player_data->subx += xm_sign * x_speed;
        player_data->suby += ym_sign * y_speed;

        i32 full_xm = player_data->subx / 128;
        i32 full_ym = player_data->suby / 128;

        player_data->subx %= 128;
        player_data->suby %= 128;

        // full-pixel movement
        if(!move_full_pixels(level, data, full_xm, full_ym)) {
            // the player was blocked: there was a collision
            player_data->xm = 0;
            player_data->ym = 0;
        }
    }
}

// draw 'count' letters around the center (xc, yc)
static inline u32 draw_letters(u32 count, i32 xc, i32 yc,
                               u32 used_sprites) {
    // make sure that 'count' is less than the limit
    if(count > LETTERS_LIMIT)
        count = LETTERS_LIMIT;

    // make sure that all sprites can be drawn
    if(count > SPRITE_COUNT - used_sprites)
        count = SPRITE_COUNT - used_sprites;

    for(u32 i = 0; i < count; i++) {
        // calculate target location
        u16 angle = letters[i].angle - tick_count * 256;

        i32 xd = (letters[i].radius)     * math_cos(angle) / 0x4000;
        i32 yd = (letters[i].radius - 1) * math_sin(angle) / 0x4000;

        i32 target_x = xc + xd;
        i32 target_y = yc + yd;

        // gradually adjust letter location to make a trailing effect
        letters[i].subx = (letters[i].subx * 7 + (target_x * 256)) / 8;
        letters[i].suby = (letters[i].suby * 7 + (target_y * 256)) / 8;

        sprite_config(used_sprites++, &(struct Sprite) {
            .x = (letters[i].subx / 256) - 4,
            .y = (letters[i].suby / 256) - 4,

            .size = SPRITE_SIZE_8x8,

            .tile = 512 + 52,
            .palette = 1
        });
    }
    return count;
}

IWRAM_SECTION
static u32 player_draw(struct Level *level, struct entity_Data *data,
                       i32 x, i32 y, u32 used_sprites) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    sprite_config(used_sprites++, &(struct Sprite) {
        .x = x - 16,
        .y = y - 29,

        .size = SPRITE_SIZE_16x32,

        .tile = 512 + 0,
        .palette = 0,

        .affine = 1,
        .affine_parameter = 0,
        .double_size = 1
    });

    // fixed point numbers: 1 = 0x4000
    i32 scale_x, scale_y;

    if(player_data->animation != ANIMATION_FALL) {
        // scale vertically
        scale_x = 0x4000;
        scale_y = 0x4000 + math_max(
            math_sin(tick_count * math_brad(90) / 16) / 4, 0
        );
    } else {
        // shrink
        scale_x = scale_y = 0x4000 - (
            0x3fff * player_data->animation_stage / FALL_ANIMATION_TIME
        );
    }

    if(player_data->sprite_flip)
        scale_x *= -1;

    sprite_affine(0, (i16 [4]) {
        256 * 0x4000 / scale_x, 0,
        0, 256 * 0x4000 / scale_y
    });

    const u32 letter_sprites = draw_letters(
        level->letters_to_deliver, x, y, used_sprites
    );

    return 1 + letter_sprites;
}

const struct entity_Type entity_player = {
    .xr = 8,
    .yr = 8,

    .is_solid = true,

    .tick = player_tick,
    .draw = player_draw,
};

// initialize data used to draw letters
static inline void init_letter_draw_data(void) {
    for(u32 i = 0; i < LETTERS_LIMIT; i++) {
        letters[i].angle = math_brad(360) * i / LETTERS_LIMIT;

        letters[i].radius = 7 + random(3);
    }

    // shuffle angle offsets
    for(u32 i = 0; i < LETTERS_LIMIT; i++) {
        u32 other = random(LETTERS_LIMIT);

        u16 tmp = letters[i].angle;
        letters[i].angle = letters[other].angle;
        letters[other].angle = tmp;
    }
}

bool level_add_player(struct Level *level) {
    level_EntityID id = level_new_entity(level);
    if(id == LEVEL_NO_ENTITY)
        return false;

    // set generic entity data
    struct entity_Data *data = &level->entities[id];

    data->x = (level->metadata->spawn.x << LEVEL_TILE_SIZE) + 8;
    data->y = 0; // TODO make it -8

    // set specific player data
    struct player_Data *player_data = (struct player_Data *) &data->data;

    set_animation(data, ANIMATION_SPAWN);

    init_letter_draw_data();

    level_add_entity(level, ENTITY_PLAYER, id);
    return true;
}
