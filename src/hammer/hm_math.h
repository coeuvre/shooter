#ifndef HM_MATH_H
#define HM_MATH_H

typedef union {
    struct {
        f32 x;
        f32 y;
    };

    struct {
        f32 u;
        f32 v;
    };
} HMV2;

static inline HMV2
hm_v2(f32 x, f32 y) {
    HMV2 result;

    result.x = x;
    result.y = y;

    return result;
}

static inline HMV2
hm_v2_zero() {
    return hm_v2(0.0f, 0.0f);
}

static inline HMV2
hm_v2_add(HMV2 a, HMV2 b) {
    HMV2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

static inline HMV2
hm_v2_sub(HMV2 a, HMV2 b) {
    HMV2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

static inline HMV2
hm_v2_mul(f32 a, HMV2 b) {
    HMV2 result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

static inline f32
hm_v2_dot(HMV2 a, HMV2 b) {
    f32 result = a.x * b.x + a.y * b.y;

    return result;
}

static inline f32
hm_get_v2_len_sq(HMV2 v) {
    f32 result = hm_v2_dot(v, v);

    return result;
}

typedef struct {
    i32 exist;
    f32 x;
    f32 y;
} HMLinearSystem2Solution;

static inline HMLinearSystem2Solution
hm_solve_linear_system2(HMV2 a, HMV2 b, HMV2 c) {
    HMLinearSystem2Solution result = {};

    f32 d = a.x * b.y - a.y * b.x;

    if (d != 0.0f) {
        result.exist = 1;
        HMV2 solution = hm_v2((c.x * b.y - c.y * b.x) / d,
                              (a.x * c.y - a.y * c.x) / d);
        result.x = solution.x;
        result.y = solution.y;
    }

    return result;
}

typedef struct {
    HMV2 a;
    HMV2 b;
} HMLine2;

static inline HMLine2
hm_line2(HMV2 a, HMV2 b) {
    HMLine2 result;

    result.a = a;
    result.b = b;

    return result;
}

typedef struct {
    HMV2 o;
    HMV2 d;
} HMRay2;

typedef struct {
    i32 exist;
    f32 t;
} HMIntersection;

static inline HMIntersection
hm_ray2_line2_intersection_test(HMRay2 ray, HMLine2 line) {
    HMIntersection result = {};

    HMV2 a = ray.d;
    HMV2 b = hm_v2_sub(line.a, line.b);
    HMV2 c = hm_v2_sub(line.a, ray.o);

    HMLinearSystem2Solution solution = hm_solve_linear_system2(a, b, c);
    if (solution.exist &&
        solution.y >= 0 && solution.y <= 1.0 &&
        solution.x >= 0.0)
    {
        result.exist = 1;
        result.t = solution.x;
    }

    return result;
}

static inline HMRay2
hm_ray2(HMV2 o, HMV2 d) {
    HMRay2 result;

    result.o = o;
    result.d = d;

    return result;
}

typedef struct {
    HMV2 min;
    HMV2 max;
} HMBBox2;

static inline HMBBox2
hm_bbox2(HMV2 min, HMV2 max) {
    HMBBox2 result;

    result.min = min;
    result.max = max;

    return result;
}

static inline HMBBox2
hm_bbox2_min_size(HMV2 min, HMV2 size) {
    HMBBox2 result;

    result.min = min;
    result.max = hm_v2_add(min, size);

    return result;
}

static inline HMBBox2
hm_bbox2_cen_size(HMV2 cen, HMV2 size) {
    HMBBox2 result;

    HMV2 halfsize = hm_v2_mul(0.5f, size);
    result.min = hm_v2_sub(cen, halfsize);
    result.max = hm_v2_add(cen, halfsize);

    return result;
}

static inline HMV2
hm_get_bbox2_cen(HMBBox2 rect) {
    // rect.min + 0.5f * (rect.max - rect.min)
    HMV2 result = hm_v2_add(rect.min, hm_v2_mul(0.5f, hm_v2_sub(rect.max, rect.min)));

    return result;
}

static inline HMV2
hm_get_bbox2_size(HMBBox2 rect) {
    HMV2 result = hm_v2_sub(rect.max, rect.min);

    return result;
}

typedef union {
    struct {
        f32 x;
        f32 y;
        f32 z;
    };

    struct {
        f32 r;
        f32 g;
        f32 b;
    };
} HMV3;

typedef union {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    struct {
        union {
            struct {
                f32 r;
                f32 g;
                f32 b;
            };

            HMV3 rgb;
        };

        f32 a;
    };
} HMV4;

static inline HMV4
hm_v4(f32 x, f32 y, f32 z, f32 w) {
    HMV4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

#endif
