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

#define HM_PIXEL_RED_MASK 0xFF000000
#define HM_PIXEL_RED_SHIFT 24

#define HM_PIXEL_GREEN_MASK 0x00FF0000
#define HM_PIXEL_GREEN_SHIFT 16

#define HM_PIXEL_BLUE_MASK 0x0000FF00
#define HM_PIXEL_BLUE_SHIFT 8

#define HM_PIXEL_ALPHA_MASK 0x000000FF
#define HM_PIXEL_ALPHA_SHIFT 0

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

void hm_draw_bbox2(HMTexture *texture, HMBBox2 bbox, HMV4 color);

#endif
