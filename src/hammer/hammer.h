#ifndef HAMMER_H
#define HAMMER_H

#include <SDL2/SDL.h>
#undef main

#define HM_LOG_ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef intptr_t isize;
typedef uintptr_t usize;
typedef float f32;
typedef double f64;

#define HM_ARRAY_COUNT(a) ((usize)(sizeof(a) / sizeof(*(a))))
#define HM_ARRAY_FOR(a, e) for (usize e = 0; e < HM_ARRAY_COUNT(a); ++e)
#define HM_SWAP(T, a, b) do { T __hm_swap_temp = a; a = b; b = __hm_swap_temp; } while (0)

#include "hm_math.h"
#include "hm_memory.h"
#include "hm_renderer.h"
#include "hm_input.h"

typedef struct {
    char *title;
    i32 width;
    i32 height;
} HM_WindowConfig;

typedef struct {
    HM_WindowConfig window;
    usize perm_memory_size;
    usize tran_memory_size;
    bool is_exit_on_esc;
} HM_Config;

#define HM_CONFIG_DEF(name) void name(HM_Config *config)
#define HM_CONFIG HM_CONFIG_DEF(hm_config)
typedef HM_CONFIG_DEF(HM_ConfigFunc);

#define HM_INIT_GAME_DEF(name) void name(HM_GameMemory *memory)
#define HM_INIT_GAME HM_INIT_GAME_DEF(hm_init_game)
typedef HM_INIT_GAME_DEF(HM_InitGameFunc);

#define HM_UPDATE_AND_RENDER_DEF(name) void name(HM_GameMemory *memory, HM_Input *input, HM_Texture2 *framebuffer)
#define HM_UPDATE_AND_RENDER HM_UPDATE_AND_RENDER_DEF(hm_update_and_render)
typedef HM_UPDATE_AND_RENDER_DEF(HM_UpdateAndRenderFunc);

#endif
