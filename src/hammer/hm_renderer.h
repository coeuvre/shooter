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
    SDL_Renderer *renderer;
    SDL_Texture *texture;
} HMRenderer;

static inline void
hm_clear_texture(HMTexture *texture) {
    memset(texture->pixels, 0,
           sizeof(*texture->pixels) * texture->width * texture->height);
}

void hm_draw_bbox2(HMTexture *texture, HMBBox2 bbox, HMV4 color);

#endif
