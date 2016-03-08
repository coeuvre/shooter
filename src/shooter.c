#include "hammer/hammer.h"

#define MAX_ENTITY_COUNT 1024
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540
#define METERS_TO_PIXELS 32
#define PIXELS_TO_METERS (1.0f / METERS_TO_PIXELS)

typedef enum {
    EntityType_Null = 0,
    EntityType_Space,
    EntityType_Ground,
    EntityType_Rope,
    EntityType_Shooter,
    EntityType_Arrow,
    EntityType_Target,
} EntityType;

typedef enum {
    TargetType_Walking,
    TargetType_Flying,
} TargetType;

enum {
    EntityFlag_Removed = (1 << 0),
    EntityFlag_Collide = (1 << 1),
};

typedef struct {
    u32 index;
    EntityType type;
    u32 flags;
    HM_V2 pos;
    HM_V2 size;
    HM_V2 vel;

    // For arrow
    f32 lifetime;
    bool is_on_ground;

    // For target
    TargetType target_type;
    HM_V2 original_pos;
    f32 flying_height_delta;
} Entity;

static inline void
set_entity_flags(Entity *entity, u32 flags) {
    entity->flags |= flags;
}

static inline bool
is_entity_flags_set(Entity *e, u32 flags) {
    bool result = e->flags & flags;
    return result;
}

static inline HM_BBox2
get_entity_bbox(Entity *entity) {
    return hm_bbox2_cen_size(entity->pos, entity->size);
}

typedef struct {
    Entity entities[MAX_ENTITY_COUNT];
    u32 entity_count;

    u32 free_entities[MAX_ENTITY_COUNT];
    u32 free_entity_count;

    Entity *space;
    Entity *rope;
    Entity *shooter;

    HM_V2 mouse_pos;

    f32 time;
    bool paused;

    bool is_player_charging;
    f32 player_charged_power;

    HM_Texture2 *hero;
} GameState;

static void
init_game_state(HM_GameMemory *memory, GameState *gamestate) {
    // The entity index 0 is considerd null entity
    for (u32 free_entity_index = MAX_ENTITY_COUNT - 1; free_entity_index != 0; --free_entity_index) {
        gamestate->free_entities[gamestate->free_entity_count++] = free_entity_index;
    }

    gamestate->hero = hm_load_bitmap(&memory->perm, "asset/hero.bmp");
}

static Entity *
add_entity(GameState *gamestate, EntityType type, HM_V2 pos) {
    assert(gamestate->entity_count < HM_ARRAY_COUNT(gamestate->entities));
    assert(gamestate->free_entity_count > 0);

    u32 entity_index = gamestate->free_entities[--gamestate->free_entity_count];
    if (entity_index >= gamestate->entity_count) {
        gamestate->entity_count = entity_index + 1;
    }

    Entity *entity = gamestate->entities + entity_index;
    *entity = (Entity){
        .index = entity_index,
        .type = type,
        .pos = pos,
    };

    return entity;
}

static void
remove_entity(GameState *gamestate, Entity *entity) {
    assert(gamestate->free_entity_count < MAX_ENTITY_COUNT);

    gamestate->free_entities[gamestate->free_entity_count++] = entity->index;

    set_entity_flags(entity, EntityFlag_Removed);
}

static Entity *
add_space(GameState *gamestate, HM_BBox2 bbox) {
    Entity *space = add_entity(gamestate, EntityType_Space, hm_get_bbox2_cen(bbox));
    space->size = hm_get_bbox2_size(bbox);
    return space;
}

static Entity *
add_ground(GameState *gamestate, HM_BBox2 bbox) {
    Entity *ground = add_entity(gamestate, EntityType_Ground, hm_get_bbox2_cen(bbox));
    ground->size = hm_get_bbox2_size(bbox);
    return ground;
}

static Entity *
add_rope(GameState *gamestate, HM_BBox2 bbox) {
    Entity *rope = add_entity(gamestate, EntityType_Rope, hm_get_bbox2_cen(bbox));
    rope->size = hm_get_bbox2_size(bbox);
    return rope;
}

static Entity *
add_shooter(GameState *gamestate, HM_V2 pos) {
    Entity *shooter = add_entity(gamestate, EntityType_Shooter, pos);
    shooter->size = hm_v2(0.6f, 1.4f);
    return shooter;
}

static Entity *
add_arrow(GameState *gamestate, HM_V2 pos, HM_V2 vel) {
    Entity *arrow = add_entity(gamestate, EntityType_Arrow, pos);
    arrow->size = hm_v2(0.08f, 0.08f);
    arrow->vel = vel;
    arrow->lifetime = 2.0f;
    return arrow;
}

static Entity *
add_walking_target(GameState *gamestate) {
    Entity *target = add_entity(gamestate, EntityType_Target, hm_v2(28, 1.5));
    target->size = hm_v2(0.6f, 1.2f);
    target->vel = hm_v2(-2, 0);
    target->target_type = TargetType_Walking;
    return target;
}

static Entity *
add_flying_target(GameState *gamestate) {
    Entity *target = add_entity(gamestate, EntityType_Target, hm_v2(28, 9));
    target->size = hm_v2(0.6f, 1.2f);
    target->vel = hm_v2(-2, 0);
    target->target_type = TargetType_Flying;
    target->original_pos = target->pos;
    target->flying_height_delta = 2;
    return target;
}

static void
build_game_scene(GameState *gamestate) {
    gamestate->space = add_space(gamestate, hm_bbox2_min_size(hm_v2(-10, -10), hm_v2(60, 30)));

    add_ground(gamestate, hm_bbox2_min_size(hm_v2(-10, 0), hm_v2(60, 1)));

    gamestate->rope = add_rope(gamestate, hm_bbox2(hm_v2(1, 8), hm_v2(1 + 0.05, 28)));

    HM_BBox2 bbox_rope = get_entity_bbox(gamestate->rope);
    gamestate->shooter = add_shooter(gamestate, hm_v2(gamestate->rope->pos.x, bbox_rope.min.y));

    add_walking_target(gamestate);
    add_flying_target(gamestate);
}

HM_CONFIG {
    config->window.title = "Shooter";
    config->window.width = WINDOW_WIDTH;
    config->window.height = WINDOW_HEIGHT;
    config->perm_memory_size = HM_MB(64);
    config->tran_memory_size = HM_MB(128);
    config->is_exit_on_esc = true;
}

HM_INIT_GAME {
    GameState *gamestate = HM_PUSH_STRUCT(&memory->perm, GameState);
    init_game_state(memory, gamestate);
    build_game_scene(gamestate);
}

HM_UPDATE_AND_RENDER {
    f32 dt = input->dt;

    GameState *gamestate = memory->perm.base;

    gamestate->time += dt;

    if (input->keyboard.keys[HM_Key_P].is_pressed) {
        gamestate->paused = !gamestate->paused;
    }

    if (gamestate->paused) {
        return;
    }

    hm_clear_texture(framebuffer);

    if (input->keyboard.keys[HM_Key_W].is_down) {
        gamestate->rope->vel.y = 10;
    } else if (input->keyboard.keys[HM_Key_S].is_down) {
        gamestate->rope->vel.y = -10;
    } else {
        gamestate->rope->vel.y = 0;
    }

    {
        HM_V2 mouse_pos_screen = hm_v2(input->mouse.x, WINDOW_HEIGHT - input->mouse.y);
        gamestate->mouse_pos = hm_v2_mul(PIXELS_TO_METERS, mouse_pos_screen);
    }

    if (input->mouse.left.is_down || input->keyboard.keys[HM_Key_SPACE].is_down) {
        gamestate->is_player_charging = true;
    } else {
        if (gamestate->is_player_charging) {
            HM_V2 arrow_dir = hm_v2_normalize(hm_v2_sub(gamestate->mouse_pos, gamestate->shooter->pos));
            f32 arrow_speed = 30 * gamestate->player_charged_power;
            add_arrow(gamestate, gamestate->shooter->pos, hm_v2_mul(arrow_speed, arrow_dir));
        }
        gamestate->is_player_charging = false;
        gamestate->player_charged_power = 0.0f;
    }

    if (gamestate->is_player_charging) {
        gamestate->player_charged_power += 0.05;
        if (gamestate->player_charged_power > 1.0f) {
            gamestate->player_charged_power = 1.0f;
        }
    }

    for (usize entity_index = 0; entity_index < gamestate->entity_count; ++entity_index) {
        Entity *entity = gamestate->entities + entity_index;

        if (is_entity_flags_set(entity, EntityFlag_Removed)) {
            continue;
        }

        // Move entity
        switch (entity->type) {
            case EntityType_Arrow: {
                if (!entity->is_on_ground) {
                    // Apply gravity
                    entity->vel = hm_v2_add(entity->vel, hm_v2(0, -9.8f * dt));

                    f32 min_t = 1.0;
                    HM_V2 delta_pos = hm_v2_mul(dt, entity->vel);
                    Entity *hit_entity = 0;

                    if (hm_get_v2_len_sq(delta_pos) > 0.0f) {
                        for (usize test_entity_index = 0; test_entity_index < gamestate->entity_count; ++test_entity_index) {
                            if (test_entity_index == entity_index) {
                                continue;
                            }

                            Entity *test_entity = gamestate->entities + test_entity_index;

                            if (!(test_entity->type == EntityType_Ground ||
                                  test_entity->type == EntityType_Target))
                            {
                                continue;
                            }

                            HM_BBox2 bound = hm_bbox2_cen_size(hm_v2_zero(), test_entity->size);
                            HM_V2 entity_pos_rel_test_entity = hm_v2_sub(entity->pos, test_entity->pos);
                            HM_Ray2 motion = hm_ray2(entity_pos_rel_test_entity, delta_pos);
                            HM_Intersection2 intersection = hm_ray2_bbox2_intersection_test(motion, bound);
                            if (intersection.exist && intersection.t < min_t) {
                                min_t = intersection.t;
                                hit_entity = test_entity;
                            }
                        }


                        entity->pos = hm_v2_add(entity->pos, hm_v2_mul(min_t, delta_pos));

                        if (hit_entity) {
                            if (hit_entity->type == EntityType_Ground) {
                                entity->is_on_ground = true;
                            } else {
                                // hit_entity->type == EntityType_Target
                                remove_entity(gamestate, hit_entity);
                                remove_entity(gamestate, entity);
                            }
                        }
                    }
                }
            } break;

            default: {
                entity->pos = hm_v2_add(entity->pos, hm_v2_mul(dt, entity->vel));
            } break;
        }

        // Entity update logic
        switch (entity->type) {
            case EntityType_Rope: {
                Entity *rope = entity;

                HM_BBox2 bbox_rope = get_entity_bbox(rope);

                // Limit the rope position
                if (bbox_rope.min.y <= 2.0f) {
                    bbox_rope.min.y = 2.0f;
                }
                if (bbox_rope.min.y >= 15.0f) {
                    bbox_rope.min.y = 15.0f;
                }

                bbox_rope = hm_bbox2_min_size(bbox_rope.min, rope->size);
                rope->pos = hm_get_bbox2_cen(bbox_rope);
            } break;

            case EntityType_Shooter: {
                Entity *shooter = entity;
                Entity *rope = gamestate->rope;
                HM_BBox2 bbox_rope = get_entity_bbox(rope);
                shooter->pos.y = bbox_rope.min.y;
            } break;

            case EntityType_Arrow: {
                if (entity->is_on_ground) {
                    entity->lifetime -= dt;
                }

                HM_BBox2 bbox_space = get_entity_bbox(gamestate->space);
                if (entity->lifetime <= 0.0f || !hm_is_bbox2_contains_point(bbox_space, entity->pos)) {
                    remove_entity(gamestate, entity);
                }
            } break;

            case EntityType_Target: {
                if (entity->target_type == TargetType_Flying) {
                    entity->pos.y = sin(2 * gamestate->time) * entity->flying_height_delta + entity->original_pos.y;
                }
            } break;

            default: break;
        }

        // Render entity
        switch (entity->type) {
            case EntityType_Space: {
                // DO NOT DRAW SPACE
            } break;

            case EntityType_Arrow: {
                i32 arrow_len = METERS_TO_PIXELS;
                f32 arrow_size = 4;
                HM_V2 arrow_pos_screen = hm_v2_mul(METERS_TO_PIXELS, entity->pos);
                HM_V2 arrow_dir = entity->vel;
                HM_Transform2 transform = hm_transform2_rotation(hm_get_v2_rad(arrow_dir));
                transform = hm_transform2_translate_by(transform,
                                                       arrow_pos_screen.x,
                                                       arrow_pos_screen.y);
                hm_draw_bbox2(framebuffer, transform,
                              hm_bbox2_min_size(hm_v2(-arrow_len, -arrow_size / 2.0f), hm_v2(arrow_len, arrow_size / 2.0f)),
                              hm_v4(0.0f, 1.0f, 0.0f, 1.0f));
            } break;

            default: {
                HM_V2 pos_screen = hm_v2_mul(METERS_TO_PIXELS, entity->pos);
                HM_V2 size_screen = hm_v2_mul(METERS_TO_PIXELS, entity->size);

                hm_draw_bbox2(framebuffer, hm_transform2_identity(),
                              hm_bbox2_cen_size(pos_screen, size_screen),
                              hm_v4(1.0f, 1.0f, 1.0f, 1.0f));
            } break;
        }
    }

    {
        i32 arrow_len = METERS_TO_PIXELS;
        HM_V2 arrow_dir = hm_v2_sub(gamestate->mouse_pos, gamestate->shooter->pos);

        HM_V2 shooter_pos_screen = hm_v2_mul(METERS_TO_PIXELS, gamestate->shooter->pos);
        i32 xoffset = -gamestate->player_charged_power * 0.4 * METERS_TO_PIXELS;

        HM_Transform2 transform = hm_transform2_rotation(hm_get_v2_rad(arrow_dir));
        transform = hm_transform2_translate_by(transform,
                                               shooter_pos_screen.x,
                                               shooter_pos_screen.y);
        hm_draw_bbox2(framebuffer, transform,
                      hm_bbox2_min_size(hm_v2(xoffset, -2), hm_v2(arrow_len, 2)),
                      hm_v4(0.0f, 1.0f, 0.0f, 1.0f));
    }

    hm_draw_bitmap(framebuffer, gamestate->hero);

#if 0
    {
        HM_Basis2 basis;
        basis.origin = hm_v2(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
        basis.xaxis = hm_v2(cos(gamestate->time), sin(gamestate->time));
        basis.yaxis = hm_v2_perp(basis.xaxis);
        hm_draw_bbox2(framebuffer, basis,
                      hm_bbox2_cen_size(hm_v2(0, 0), hm_v2(512, 512)),
                      hm_v4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    printf("entity count: %d\n", gamestate->entity_count);
#endif
}

#define HM_STATIC
#include "hammer/hammer.c"
