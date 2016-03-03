#include "hammer/hammer.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540
#define METERS_TO_PIXELS 32

typedef enum {
    EntityType_Null = 0,
    EntityType_Rope,
    EntityType_Shooter,
    EntityType_Arrow,
    EntityType_Target,
} EntityType;

enum {
    EntityFlag_Removed = (1 << 0),
    EntityFlag_Collide = (1 << 1),
};

typedef struct {
    u32 index;
    EntityType type;
    u32 flags;
    HMV2 pos;
    HMV2 size;
    HMV2 vel;
} Entity;

typedef struct {
    Entity entities[1024];
    usize entity_count;

    Entity *shooter;
    Entity *rope;

    HMV2 mouse_pos;
} GameState;

static Entity *
add_entity(GameState *gamestate, EntityType type, HMV2 pos) {
    assert(gamestate->entity_count < HM_ARRAY_COUNT(gamestate->entities));

    u32 entity_index = gamestate->entity_count++;
    Entity *entity = gamestate->entities + entity_index;
    *entity = (Entity){
        .index = entity_index,
        .type = type,
        .pos = pos,
    };

    return entity;
}

static Entity *
add_rope(GameState *gamestate, HMBBox2 bbox) {
    Entity *rope = add_entity(gamestate, EntityType_Rope, hm_get_bbox2_cen(bbox));
    rope->size = hm_get_bbox2_size(bbox);
    return rope;
}

static Entity *
add_shooter(GameState *gamestate, HMV2 pos) {
    Entity *shooter = add_entity(gamestate, EntityType_Shooter, pos);
    shooter->size = hm_v2(0.6f, 1.4f);
    return shooter;
}

static Entity *
add_arrow(GameState *gamestate, HMV2 pos, HMV2 vel) {
    Entity *arrow = add_entity(gamestate, EntityType_Arrow, pos);
    arrow->size = hm_v2(0.08f, 0.08f);
    arrow->vel = vel;
    return arrow;
}

static Entity *
add_target(GameState *gamestate, HMV2 pos) {
    Entity *shooter = add_entity(gamestate, EntityType_Target, pos);
    shooter->size = hm_v2(0.2f, 1.0f);
    return shooter;
}

static void
init_game_state(GameState *gamestate) {
    // Add a null entity at index 0
    add_entity(gamestate, EntityType_Null, hm_v2(0, 0));

    gamestate->rope = add_rope(gamestate, hm_bbox2(hm_v2(1, 0),
                               hm_v2(1 + 0.05, 16.875)));

    gamestate->shooter = add_shooter(gamestate, hm_v2(1, 10));

    add_target(gamestate, hm_v2(28, 9));
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
    init_game_state(gamestate);
}

HM_UPDATE_AND_RENDER {
    f32 dt = input->dt;

    hm_clear_texture(framebuffer);

    GameState *gamestate = memory->perm.base;

    if (input->keyboard.keys[HMKey_W].is_down) {
        gamestate->shooter->vel.y = 10;
    } else if (input->keyboard.keys[HMKey_S].is_down) {
        gamestate->shooter->vel.y = -10;
    } else {
        gamestate->shooter->vel.y = 0;
    }

    gamestate->mouse_pos = hm_v2(input->mouse.x, WINDOW_HEIGHT - input->mouse.y);

    if (input->keyboard.keys[HMKey_SPACE].is_down) {
        add_arrow(gamestate, gamestate->shooter->pos, hm_v2(20, 15));
    }

    for (usize entity_index = 0; entity_index < gamestate->entity_count; ++entity_index) {
        Entity *entity = gamestate->entities + entity_index;

        entity->pos = hm_v2_add(entity->pos, hm_v2_mul(dt, entity->vel));

        switch (entity->type) {
            case EntityType_Shooter: {
                Entity *shooter = entity;
                Entity *rope = gamestate->rope;
                HMBBox2 rope_bbox = hm_bbox2_cen_size(rope->pos, rope->size);
                if (shooter->pos.y > rope_bbox.max.y) {
                    shooter->pos.y = rope_bbox.max.y;
                }
                if (shooter->pos.y < rope_bbox.min.y) {
                    shooter->pos.y = rope_bbox.min.y;
                }
            } break;

            case EntityType_Arrow: {
                // Apply gravity
                entity->vel = hm_v2_add(entity->vel, hm_v2(0, -9.8f * dt));
            };

            default: break;
        }

        HMV2 pos_screen = hm_v2_mul(METERS_TO_PIXELS, entity->pos);
        HMV2 size_screen = hm_v2_mul(METERS_TO_PIXELS, entity->size);
        hm_draw_bbox2(framebuffer, hm_bbox2_cen_size(pos_screen, size_screen),
                      hm_v4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    hm_draw_bbox2(framebuffer, hm_bbox2_cen_size(gamestate->mouse_pos, hm_v2(4, 4)),
                  hm_v4(1.0f, 1.0f, 1.0f, 1.0f));
}

#define HM_STATIC
#include "hammer/hammer.c"
