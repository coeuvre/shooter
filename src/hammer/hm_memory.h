#ifndef HM_MEMORY_H
#define HM_MEMORY_H


#if !defined(offsetof)
#  define offsetof(type, member) ((usize)&(((type *)0)->member))
#endif

#define HM_KB(x) ((x) * 1024LL)
#define HM_MB(x) (HM_KB(x) * 1024LL)
#define HM_GB(x) (HM_MB(x) * 1024LL)
#define HM_TB(x) (HM_TB(x) * 1024LL)

typedef struct {
    void *base;
    usize size;
    usize used;
} HMMemoryArena;

typedef struct {
    HMMemoryArena perm;
    HMMemoryArena tran;
} HMGameMemory;


#define HM_PUSH_STRUCT(arena, type) ((type *)hm_push_size(arena, sizeof(type)))
#define HM_PUSH_ARRAY(arena, count, type) ((type *)hm_push_size(arena, (count) * sizeof(type)))
#define HM_PUSH_SIZE(arena, size) hm_push_size(arena, (size))

static inline void *
hm_push_size(HMMemoryArena *arena, usize size) {
    assert(arena->used + size <= arena->size);

    void *result = (u8 *)arena->base + arena->used;
    arena->used += size;

    return result;
}

#endif
