#ifndef HM_RENDERER_H
#define HM_RENDERER_H

typedef struct {
    // The pixels is in sRGB color space
    //
    // Bit pattern: 0xRRGGBBAA
    u32 *pixels;

    i32 width;
    i32 height;
    i32 pitch;
} HM_Texture;

#if 0
typedef struct {
} HM_RenderCommand;

typedef struct {
} HM_RenderCommandBuffer;

typedef struct {
} HM_RenderContext;
#endif

typedef struct {
    HM_V2 origin;
    HM_V2 xaxis;
    HM_V2 yaxis;
} HM_Basis2;

static inline HM_Basis2
hm_basis2_identity() {
    HM_Basis2 result;

    result.origin = hm_v2_zero();
    result.xaxis = hm_v2(1, 0);
    result.yaxis = hm_v2(0, 1);

    return result;
}

typedef struct {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} HM_Renderer;

static inline void
hm_clear_texture(HM_Texture *texture) {
    memset(texture->pixels, 0,
           sizeof(*texture->pixels) * texture->width * texture->height);
}

void hm_draw_bbox2(HM_Texture *texture, HM_Basis2 basis, HM_BBox2 bbox, HM_V4 color);

#endif
