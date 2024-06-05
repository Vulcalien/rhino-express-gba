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

#include "input.h"
#include "sprite.h"
#include "random.h"
#include "math.h"

#include "level.h"
#include "sfx.h"

#define   MAX_SPEED (1280)
#define START_SPEED (64)

struct player_Data {
    // subpixel coordinates (128:1)
    i16 subx;
    i16 suby;

    i16 xm;
    i16 ym;

    i8 stored_xm;
    i8 stored_ym;

    bool hit_obstacle;

    u8 sprite_flip;

    u8 unused[4];
};

static_assert(
    sizeof(struct player_Data) == ENTITY_EXTRA_DATA_SIZE,
    "struct player_Data is of wrong size"
);

#define LETTERS_LIMIT (8)

static struct {
    // subpixel coordinates (256:1)
    i32 subx;
    i32 suby;

    u16 angle;
    u16 radius;
} letters[LETTERS_LIMIT];

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

static inline bool is_tile_center(i32 x, i32 y) {
    u32 tile_pixels = 1 << LEVEL_TILE_SIZE;
    return (x % tile_pixels == tile_pixels / 2 &&
            y % tile_pixels == tile_pixels / 2);
}

static inline void enter_tile(struct Level *level,
                              struct entity_Data *data,
                              i32 xt, i32 yt) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    // TODO
    switch(level_get_tile(level, xt, yt)) {
        case TILE_VOID:
        case TILE_HOLE:
            break;

        case TILE_WOOD:
            // TODO check if the player is fast enough to break the wood
            player_data->hit_obstacle = true;
            break;

        case TILE_ROCK:
            // TODO check if the player is fast enough to break the rock
            player_data->hit_obstacle = true;
            break;

        case TILE_WATER:
            player_data->xm = player_data->ym = 0;
            player_data->stored_xm = player_data->stored_ym = 0;
            // TODO add block particle
            SOUND_DMA_PLAY(sfx_water, false, SOUND_DMA_A);
            break;

        default:
            break;
    }
}

static inline void leave_tile(struct Level *level,
                              struct entity_Data *data,
                              i32 xt, i32 yt) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    switch(level_get_tile(level, xt, yt)) {
        case TILE_FALL_PLATFORM:
            level_set_tile(level, xt, yt, TILE_HOLE);
            // TODO add falling platform particle
            // TODO add falling platform sound
            break;

        default:
            break;
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
            i32 prev_xt = (data->x >> LEVEL_TILE_SIZE) - math_sign(xm);
            i32 prev_yt = (data->y >> LEVEL_TILE_SIZE) - math_sign(ym);
            leave_tile(level, data, prev_xt, prev_yt);

            i32 next_xt = data->x >> LEVEL_TILE_SIZE;
            i32 next_yt = data->y >> LEVEL_TILE_SIZE;
            enter_tile(level, data, next_xt, next_yt);

            // check if movement was canceled
            if(player_data->xm == 0 && player_data->ym == 0)
                break;

            // check if an obstacle was hit
            if(player_data->hit_obstacle) {
                player_data->xm = -math_sign(player_data->xm);
                player_data->ym = -math_sign(player_data->ym);

                SOUND_DMA_PLAY(sfx_hit_obstacle, false, SOUND_DMA_A);
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

    // if the level is still in editing mode, do nothing
    if(level->is_editing)
        return;

    // process player input
    read_input(&player_data->stored_xm, &player_data->stored_ym);

    if(player_data->xm == 0 && player_data->ym == 0) {
        player_data->xm = player_data->stored_xm;
        player_data->ym = player_data->stored_ym;

        player_data->stored_xm = player_data->stored_ym = 0;

        // update sprite flip bit
        if(player_data->xm < 0)
            player_data->sprite_flip = 0;
        else if(player_data->xm > 0)
            player_data->sprite_flip = 1;
    } else if(player_data->hit_obstacle) {
        // if an obstacle was hit, gradually move back by one tile
        entity_move(
            level, data,
            player_data->xm * 2, player_data->ym * 2
        );

        static i8 last_rumble_offset = 0;

        if(is_tile_center(data->x, data->y)) {
            player_data->xm = player_data->ym = 0;
            player_data->hit_obstacle = false;

            // restore initial level offset
            level->offset.y -= last_rumble_offset;
            last_rumble_offset = 0;
        } else {
            if(tick_count % 2 == 0) {
                // apply a new level offset
                level->offset.y -= last_rumble_offset;
                last_rumble_offset = -2 + rand() % 5;
                level->offset.y += last_rumble_offset;
            }
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

static inline u32 calculate_inverse_y_scale(u32 arg) {
    arg *= MATH_PI / 32;
    u32 val = math_max(math_sin(arg), 0) * 43 / 0x4000;
    return 256 - val;
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

    struct Sprite sprite = {
        .tile = 52 * 2,
        .color_mode = 1
    };

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

        sprite.x = (letters[i].subx / 256) - 4;
        sprite.y = (letters[i].suby / 256) - 4;
        sprite_config(used_sprites++, &sprite);
    }
    return count;
}

IWRAM_SECTION
static u32 player_draw(struct Level *level, struct entity_Data *data,
                       i32 x, i32 y, u32 used_sprites) {
    struct player_Data *player_data = (struct player_Data *) &data->data;

    // 8-bit fixed point decimal (1 = 256)
    u32 inverse_y_scale = calculate_inverse_y_scale(tick_count);

    struct Sprite sprite = {
        .x = x - 16,
        .y = y - 4 - (256 * 16) / inverse_y_scale,

        .size = 1,

        .tile = 0,
        .color_mode = 1,

        .affine_transformation = 1,
        .affine_parameter = 0,
        .double_size = 1
    };
    sprite_config(used_sprites++, &sprite);

    // set OBJ parameter
    vu16 *parameters = &OAM[3];
    parameters[0]  = 256 * (player_data->sprite_flip ? -1 : +1);
    parameters[4]  = 0;
    parameters[8]  = 0;
    parameters[12] = inverse_y_scale;

    const u32 letter_sprites = draw_letters(
        level->letters_to_deliver, x, y, used_sprites
    );

    return 1 + letter_sprites;
}

IWRAM_SECTION
static bool player_touch_entity(struct Level *level,
                                struct entity_Data *data,
                                struct entity_Data *touched_data) {
    if(touched_data->type != ENTITY_MAILBOX)
        return true;

    // TODO check if a letter has to be removed
    // (should that be done here or in 'mailbox.c' ?)

    return false;
}

const struct entity_Type entity_player = {
    .xr = 8,
    .yr = 8,

    .is_solid = true,

    .tick = player_tick,
    .draw = player_draw,

    .touch_entity = player_touch_entity
};

// initialize data used to draw letters
static inline void init_letter_draw_data(void) {
    for(u32 i = 0; i < LETTERS_LIMIT; i++) {
        letters[i].angle = (MATH_PI * 2) * i / LETTERS_LIMIT;

        letters[i].radius = 7 + rand() % 3;
    }

    // shuffle angle offsets
    for(u32 i = 0; i < LETTERS_LIMIT; i++) {
        u32 other = rand() % LETTERS_LIMIT;

        u16 tmp = letters[i].angle;
        letters[i].angle = letters[other].angle;
        letters[other].angle = tmp;
    }
}

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

    player_data->sprite_flip = 0;

    init_letter_draw_data();

    level_add_entity(level, ENTITY_PLAYER, id);
    return true;
}
