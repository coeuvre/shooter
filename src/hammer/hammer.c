#include "hammer.h"

#include "hm_renderer.c"

HM_CONFIG_DEF(hm_config_stub) {
    (void)config;
}

HM_INIT_GAME_DEF(hm_init_game_stub) {
    (void)memory;
}

HM_UPDATE_AND_RENDER_DEF(hm_update_and_render_stub) {
    (void)memory;
    (void)input;
    (void)framebuffer;
}

typedef struct {
    HM_ConfigFunc *config;
    HM_InitGameFunc *init_game;
    HM_UpdateAndRenderFunc *update_and_render;
} HM_Callback;

int
main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    HM_Callback hm = {
        .config = hm_config_stub,
        .init_game = hm_init_game_stub,
        .update_and_render = hm_update_and_render_stub,
    };

#ifdef HM_STATIC
    hm.config = hm_config;
    hm.init_game = hm_init_game;
    hm.update_and_render = hm_update_and_render;
#endif

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        HM_LOG_ERROR("Failed to initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    HM_Config config = {};
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

    HM_Renderer renderer;
    renderer.renderer = sdl_renderer;
    renderer.texture = sdl_texture;

    HM_Texture framebuffer;
    framebuffer.pixels = buffer;
    framebuffer.width = config.window.width;
    framebuffer.height = config.window.height;
    framebuffer.pitch = framebuffer.width * 4;

    HM_GameMemory memory = {
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

    HM_Input input = {};
    HM_Input old_input = input;

    input.dt = 1.0f / 60.0f;
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

                default: break;
            }
        }

        //
        // Query input device
        //
        int key_count;
        const u8 *keys = SDL_GetKeyboardState(&key_count);
        for (int key_index = 0; key_index < key_count; ++key_index) {
            input.keyboard.keys[key_index].is_down = keys[key_index];

            bool key_is_pressed = keys[key_index] &&
                                  !old_input.keyboard.keys[key_index].is_down;
            input.keyboard.keys[key_index].is_pressed = key_is_pressed;
        }

        u32 mouse_button_state = SDL_GetMouseState(&input.mouse.x, &input.mouse.y);
        input.mouse.left.is_down = mouse_button_state & SDL_BUTTON(SDL_BUTTON_LEFT);
        input.mouse.left.is_pressed = input.mouse.left.is_down && !old_input.mouse.left.is_down;
        input.mouse.middle.is_down = mouse_button_state & SDL_BUTTON(SDL_BUTTON_MIDDLE);
        input.mouse.middle.is_pressed = input.mouse.middle.is_down && !old_input.mouse.middle.is_down;
        input.mouse.right.is_down = mouse_button_state & SDL_BUTTON(SDL_BUTTON_RIGHT);
        input.mouse.right.is_pressed = input.mouse.right.is_down && !old_input.mouse.right.is_down;

        if (config.is_exit_on_esc && input.keyboard.keys[HM_Key_ESCAPE].is_down) {
            quit = 1;
        }

        hm.update_and_render(&memory, &input, &framebuffer);

        //u32 frametime = SDL_GetTicks() - frame_begin;
        //printf("%u\n", frametime);

        render_to_screen(&renderer, &framebuffer);

#if 0
        frametime = SDL_GetTicks() - frame_begin;
        if (target_frametime > frametime) {
            SDL_Delay(target_frametime - frametime);
        }
#endif

        HM_SWAP(HM_Input, input, old_input);
    }

    free(memory.perm.base);
    free(memory.tran.base);
    free(buffer);

    SDL_DestroyTexture(renderer.texture);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
