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
} HMTexture;

#if 0
typedef struct {
} HMRenderCommand;

typedef struct {
} HMRenderCommandBuffer;

typedef struct {
} HMRenderContext;
#endif

typedef struct {
    HMV2 origin;
    HMV2 xaxis;
    HMV2 yaxis;
} HMBasis2;

static inline HMBasis2
hm_basis2_identity() {
    HMBasis2 result;

    result.origin = hm_v2_zero();
    result.xaxis = hm_v2(1, 0);
    result.yaxis = hm_v2(0, 1);

    return result;
}

typedef struct {
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} HMRenderer;

static inline void
hm_clear_texture(HMTexture *texture) {
    memset(texture->pixels, 0,
           sizeof(*texture->pixels) * texture->width * texture->height);
}

void hm_draw_bbox2(HMTexture *texture, HMBasis2 basis, HMBBox2 bbox, HMV4 color);

#endif
