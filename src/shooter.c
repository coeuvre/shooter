#include "hammer/hammer.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540

typedef enum {
    EntityType_Null = 0,
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
add_shooter(GameState *gamestate, HMV2 pos) {
    Entity *shooter = add_entity(gamestate, EntityType_Shooter, pos);
    shooter->size = hm_v2(20, 20);
    return shooter;
}

static void
init_game_state(GameState *gamestate) {
    // Add a null entity at index 0
    add_entity(gamestate, EntityType_Null, hm_v2(0, 0));

    add_shooter(gamestate, hm_v2(WINDOW_WIDTH / 2, 10));
}

HM_CONFIG {
    config->window.title = "Shooter";
    config->window.width = WINDOW_WIDTH;
    config->window.height = WINDOW_HEIGHT;
    config->perm_memory_size = HM_MB(64);
    config->tran_memory_size = HM_GB(1);
    config->is_exit_on_esc = true;
}

HM_INIT_GAME {
    GameState *gamestate = HM_PUSH_STRUCT(&memory->perm, GameState);
    init_game_state(gamestate);
}

HM_HANDLE_EVENT {
    (void)e;

    GameState *gamestate = HM_PUSH_STRUCT(&memory->perm, GameState);
    (void)gamestate;
}

HM_UPDATE_AND_RENDER {
    (void)dt;

    GameState *gamestate = memory->perm.base;

    for (usize entity_index = 0; entity_index < gamestate->entity_count; ++entity_index) {
        Entity *entity = gamestate->entities + entity_index;
        hm_draw_bbox2(framebuffer, hm_bbox2_cen_size(entity->pos, entity->size),
                      hm_v4(1.0f, 1.0f, 1.0f, 1.0f));
    }
}

#define HM_STATIC
#include "hammer/hammer.c"
