#define GAMMA 2.2f

#define HM_PIXEL_RED_MASK 0xFF000000
#define HM_PIXEL_RED_SHIFT 24

#define HM_PIXEL_GREEN_MASK 0x00FF0000
#define HM_PIXEL_GREEN_SHIFT 16

#define HM_PIXEL_BLUE_MASK 0x0000FF00
#define HM_PIXEL_BLUE_SHIFT 8

#define HM_PIXEL_ALPHA_MASK 0x000000FF
#define HM_PIXEL_ALPHA_SHIFT 0

static void
render_to_screen(HMRenderer *renderer, HMTexture *texture) {
    SDL_UpdateTexture(renderer->texture, 0, texture->pixels, texture->width * 4);
    SDL_RenderCopy(renderer->renderer, renderer->texture, 0, 0);
    SDL_RenderPresent(renderer->renderer);
}

static inline u32
u32_rgba(u8 r, u8 g, u8 b, u8 a) {
    u32 result = (r << HM_PIXEL_RED_SHIFT) |
                 (g << HM_PIXEL_GREEN_SHIFT) |
                 (b << HM_PIXEL_BLUE_SHIFT) |
                 (a << HM_PIXEL_ALPHA_SHIFT);
    return result;
}

static inline u32
v4_linear_to_u32_srgb(HMV4 color) {
    static u8 gamma_correction_table[256];
    static bool is_gamma_correction_table_initialized;

    if (!is_gamma_correction_table_initialized) {
        f32 inv_gamma = 1.0f / GAMMA;

        for (usize i = 0; i < HM_ARRAY_COUNT(gamma_correction_table); ++i) {
            f32 c = i / 255.0f;
            c = powf(c, inv_gamma);
            gamma_correction_table[i] = (c * 255.0f + 0.5f);
        }

        is_gamma_correction_table_initialized = 1;
    }

    u8 r = color.r * 255.0f;
    u8 g = color.g * 255.0f;
    u8 b = color.b * 255.0f;
    u8 a = color.a * 255.0f;

    u32 result = u32_rgba(gamma_correction_table[r],
                          gamma_correction_table[g],
                          gamma_correction_table[b],
                          a);

    return result;
}

void hm_draw_bbox2(HMTexture *texture, HMBBox2 bbox, HMV4 color) {
    i32 minx = (i32)bbox.min.x;
    i32 miny = (i32)bbox.min.y;
    i32 maxx = (i32)bbox.max.x;
    i32 maxy = (i32)bbox.max.y;

    if (minx < 0) { minx = 0; }
    if (maxx >= texture->width) { maxx = texture->width; }
    if (miny < 0) { miny = 0; }
    if (maxy >= texture->height) { maxy = texture->height; }

    u32 color32 = v4_linear_to_u32_srgb(color);
    u32 *row = texture->pixels + (texture->height - 1 - miny) * texture->width + minx;
    for (i32 y = miny; y < maxy; ++y) {
        u32 *pixel = row;
        for (i32 x = minx; x < maxx; ++x) {
            *pixel++ = color32;
        }
        row -= texture->width;
    }
}
