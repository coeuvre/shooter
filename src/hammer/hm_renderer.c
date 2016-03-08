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
render_to_screen(HM_Renderer *renderer, HM_Texture2 *texture) {
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
v4_linear_to_u32_srgb(HM_V4 color) {
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

static inline HM_V4
u32_srgb_to_v4_linear(u32 color) {
    static u8 gamma_table[256];
    static u32 is_gamma_table_initialized;

    if (!is_gamma_table_initialized) {
        f32 gamma = GAMMA;

        for (int i = 0; i < 256; ++i) {
            f32 c = i / 255.0f;
            c = powf(c, gamma);
            gamma_table[i] = (c * 255.0f + 0.5f);
        }

        is_gamma_table_initialized = 1;
    }

    u8 r = (color & HM_PIXEL_RED_MASK) >> HM_PIXEL_RED_SHIFT;
    u8 g = (color & HM_PIXEL_GREEN_MASK) >> HM_PIXEL_GREEN_SHIFT;
    u8 b = (color & HM_PIXEL_BLUE_MASK) >> HM_PIXEL_BLUE_SHIFT;
    u8 a = (color & HM_PIXEL_ALPHA_MASK) >> HM_PIXEL_ALPHA_SHIFT;

    HM_V4 result = hm_v4(gamma_table[r] / 255.0f,
                         gamma_table[g] / 255.0f,
                         gamma_table[b] / 255.0f,
                         a / 255.0f);

    return result;
}

static inline HM_V2
get_point_in_basis2(HM_Basis2 basis, HM_V2 point) {
    HM_V2 result;

    //result = basis.origin + point.x * basis.xaxis + point.y * basis.yaxis
    result = hm_v2_add(basis.origin,
                       hm_v2_add(
                           hm_v2_mul(point.x, basis.xaxis),
                           hm_v2_mul(point.y, basis.yaxis)
                       ));

    return result;
}

HM_Texture2 *
hm_load_bitmap(HM_MemoryArena *arena, char *path) {
    SDL_Surface *surface = SDL_LoadBMP(path);

    if (surface) {
        if (surface->format->BytesPerPixel != 4) {
            HM_LOG_ERROR("Only support BMP file with 32-bit pixel depth.");
            SDL_FreeSurface(surface);
            assert(false);
            return 0;
        }

        HM_Texture2 *result = HM_PUSH_STRUCT(arena, HM_Texture2);
        result->width = surface->w;
        result->height = surface->h;
        result->pitch = result->width * sizeof(u32);
        result->pixels = HM_PUSH_SIZE(arena, result->pitch * result->height);

        u8 *src_row = (u8 *)surface->pixels;
        u8 *dst_row = (u8 *)result->pixels;
        for (int y = 0; y < result->height; ++y) {
            u32 *src = (u32 *)src_row;
            u32 *dst = (u32 *)dst_row;
            for (int x = 0; x < result->width; ++x) {
                u32 color32 = *src++;

                u8 r = (color32 & surface->format->Rmask) >> surface->format->Rshift;
                u8 g = (color32 & surface->format->Gmask) >> surface->format->Gshift;
                u8 b = (color32 & surface->format->Bmask) >> surface->format->Bshift;
                u8 a = (color32 & surface->format->Amask) >> surface->format->Ashift;

                // pre-multiply alpha
                HM_V4 color = u32_srgb_to_v4_linear(u32_rgba(r, g, b, a));
                color.rgb = hm_v3_mul(color.a, color.rgb);

                *dst++ = v4_linear_to_u32_srgb(color);
            }
            src_row += surface->pitch;
            dst_row += result->pitch;
        }

        SDL_FreeSurface(surface);

        return result;
    } else {
        HM_LOG_ERROR("Failed to load bitmap %s: %s\n", path, SDL_GetError());
        assert(false);
        return 0;
    }
}

void
hm_draw_bitmap(HM_Texture2 *target, HM_Texture2 *bitmap) {
    u8 *src_row = (u8 *)bitmap->pixels;
    u8 *dst_row = (u8 *)target->pixels;

    for (int y = 0; y < bitmap->height; ++y) {
        u32 *src = (u32 *)src_row;
        u32 *dst = (u32 *)dst_row;

        for (int x = 0; x < bitmap->width; ++x) {
            *dst++ = *src++;
        }
        src_row += bitmap->pitch;
        dst_row += target->pitch;
    }
}

void
hm_draw_bbox2(HM_Texture2 *target, HM_Basis2 basis, HM_BBox2 bbox, HM_V4 color) {
    //
    // [3] +---+ [2]
    //     |   |
    // [0] +---+ [1]
    //
    HM_V2 points[4] = {
        get_point_in_basis2(basis, bbox.min),
        get_point_in_basis2(basis, hm_v2(bbox.max.x, bbox.min.y)),
        get_point_in_basis2(basis, bbox.max),
        get_point_in_basis2(basis, hm_v2(bbox.min.x, bbox.max.y)),
    };
    HM_V2 edges_perp[4] = {
        hm_v2_perp(hm_v2_sub(points[1], points[0])),
        hm_v2_perp(hm_v2_sub(points[2], points[1])),
        hm_v2_perp(hm_v2_sub(points[3], points[2])),
        hm_v2_perp(hm_v2_sub(points[0], points[3])),
    };

    i32 minx = target->width;
    i32 maxx = 0;
    i32 miny = target->height;
    i32 maxy = 0;

    HM_ARRAY_FOR(points, point_index) {
        HM_V2 point = points[point_index];

        i32 floorx = floorf(point.x);
        i32 ceilx = ceilf(point.x);
        i32 floory = floorf(point.y);
        i32 ceily = ceilf(point.y);

        if (floorx < minx) { minx = floorx; }
        if (ceilx > maxx) { maxx = ceilx; }
        if (floory < miny) { miny = floory; }
        if (ceily > maxy) { maxy = ceily; }
    }

    if (minx < 0) { minx = 0; }
    if (maxx >= target->width) { maxx = target->width; }
    if (miny < 0) { miny = 0; }
    if (maxy >= target->height) { maxy = target->height; }

    // pre-multiply alpha
    color.rgb = hm_v3_mul(color.a, color.rgb);

    u8 *row = (u8 *)target->pixels + (target->height - 1 - miny) * target->pitch + minx * sizeof(*target->pixels);
    for (i32 y = miny; y < maxy; ++y) {
        u32 *pixel = (u32 *)row;
        for (i32 x = minx; x < maxx; ++x) {
            bool draw = true;
            HM_V2 testp = hm_v2(x, y);
            HM_V2 tests[4] = {
                hm_v2_sub(testp, points[0]),
                hm_v2_sub(testp, points[1]),
                hm_v2_sub(testp, points[2]),
                hm_v2_sub(testp, points[3]),
            };

            HM_ARRAY_FOR(edges_perp, edge_perp_index) {
                HM_V2 edge_perp = edges_perp[edge_perp_index];
                HM_V2 testv = tests[edge_perp_index];
                bool is_inside = hm_v2_dot(testv, edge_perp) >= 0.0f;
                if (!is_inside) {
                    draw = false;
                    break;
                }
            }

            if (draw) {
                u32 color32 = v4_linear_to_u32_srgb(color);
                *pixel = color32;
            }

            ++pixel;
        }
        row -= target->pitch;
    }
}
