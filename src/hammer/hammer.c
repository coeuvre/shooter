#include "hammer.h"

#include "hm_renderer.c"

HM_CONFIG_DEF(hm_config_stub) {
    (void)config;
}

HM_INIT_GAME_DEF(hm_init_game_stub) {
    (void)memory;
}

HM_HANDLE_EVENT_DEF(hm_handle_event_stub) {
    (void)e;
    (void)memory;
}

HM_UPDATE_AND_RENDER_DEF(hm_update_and_render_stub) {
    (void)memory;
    (void)dt;
    (void)framebuffer;
}

typedef struct {
    HMConfigFunc *config;
    HMInitGameFunc *init_game;
    HMHandleEventFunc *handle_event;
    HMUpdateAndRenderFunc *update_and_render;
} HMCallback;

int
main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    HMCallback hm = {
        .config = hm_config_stub,
        .init_game = hm_init_game_stub,
        .handle_event = hm_handle_event_stub,
        .update_and_render = hm_update_and_render_stub,
    };

#ifdef HM_STATIC
    hm.config = hm_config;
    hm.init_game = hm_init_game;
    hm.handle_event = hm_handle_event;
    hm.update_and_render = hm_update_and_render;
#endif

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        HM_LOG_ERROR("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    HMConfig config = {};
    hm.config(&config);

    // On Apple's OS X you must set the NSHighResolutionCapable
    // Info.plist property to YES, otherwise you will not receive a
    // High DPI OpenGL canvas.
    SDL_Window *window = SDL_CreateWindow(config.window.title,
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          config.window.width,
                                          config.window.height,
                                          SDL_WINDOW_OPENGL);
    if (!window) {
        HM_LOG_ERROR("Failed to create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(window, -1,
                                                    SDL_RENDERER_ACCELERATED |
                                                    SDL_RENDERER_PRESENTVSYNC);
    if (!sdl_renderer) {
        HM_LOG_ERROR("Failed to create renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture *sdl_texture = SDL_CreateTexture(sdl_renderer,
                                                 SDL_PIXELFORMAT_RGBA8888,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 config.window.width, config.window.height);

    u32 *buffer = calloc(config.window.width * config.window.height, sizeof(u32));

    HMRenderer renderer;
    renderer.renderer = sdl_renderer;
    renderer.texture = sdl_texture;

    HMTexture framebuffer;
    framebuffer.pixels = buffer;
    framebuffer.width = config.window.width;
    framebuffer.height = config.window.height;
    framebuffer.pitch = framebuffer.width * 4;

    HMGameMemory memory = {
        .perm = {
            .base = calloc(config.perm_memory_size, sizeof(u8)),
            .size = config.perm_memory_size,
            .used = 0,
        },

        .tran = {
            .base = calloc(config.tran_memory_size, sizeof(u8)),
            .size = config.tran_memory_size,
            .used = 0,
        },
    };

    if (hm.init_game) {
        hm.init_game(&memory);
    }

    f32 dt = 1.0f / 60.0f;
    //u32 target_frametime = dt * 1000.0f;

    i32 quit = 0;
    while (!quit) {
        //u32 frame_begin = SDL_GetTicks();

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    quit = 1;
                } break;

                case SDL_KEYDOWN: {
                    if (e.key.keysym.sym == SDLK_ESCAPE && config.is_exit_on_esc) {
                        quit = 1;
                    }
                } break;

                default: break;
            }

            hm.handle_event(&e, &memory);
        }

        hm.update_and_render(&memory, dt, &framebuffer);

        //u32 frametime = SDL_GetTicks() - frame_begin;
        //printf("%u\n", frametime);

        render_to_screen(&renderer, &framebuffer);

#if 0
        frametime = SDL_GetTicks() - frame_begin;
        if (target_frametime > frametime) {
            SDL_Delay(target_frametime - frametime);
        }
#endif
    }

    free(memory.perm.base);
    free(memory.tran.base);
    free(buffer);

    SDL_DestroyTexture(renderer.texture);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
