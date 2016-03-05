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
render_to_screen(HM_Renderer *renderer, HM_Texture *texture) {
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

void
hm_draw_bbox2(HM_Texture *texture, HM_Basis2 basis, HM_BBox2 bbox, HM_V4 color) {
    (void)basis;

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

    i32 minx = texture->width;
    i32 maxx = 0;
    i32 miny = texture->height;
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
    if (maxx >= texture->width) { maxx = texture->width; }
    if (miny < 0) { miny = 0; }
    if (maxy >= texture->height) { maxy = texture->height; }

    u32 color32 = v4_linear_to_u32_srgb(color);
    u8 *row = (u8 *)texture->pixels + (texture->height - 1 - miny) * texture->pitch + minx * sizeof(*texture->pixels);
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
                HM_V2 *edge_perp = edges_perp + edge_perp_index;
                HM_V2 *testv = tests + edge_perp_index;
                bool is_inside = hm_v2_dot(*testv, *edge_perp) >= 0.0f;
                if (!is_inside) {
                    draw = false;
                    break;
                }
            }

            if (draw) {
                *pixel = color32;
            }

            ++pixel;
        }
        row -= texture->pitch;
    }
}
