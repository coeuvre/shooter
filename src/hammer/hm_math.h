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
} HM_V2;

static inline HM_V2
hm_v2(f32 x, f32 y) {
    HM_V2 result;

    result.x = x;
    result.y = y;

    return result;
}

static inline HM_V2
hm_v2_zero() {
    return hm_v2(0.0f, 0.0f);
}

static inline HM_V2
hm_v2_add(HM_V2 a, HM_V2 b) {
    HM_V2 result;

    result.x = a.x + b.x;
    result.y = a.y + b.y;

    return result;
}

static inline HM_V2
hm_v2_sub(HM_V2 a, HM_V2 b) {
    HM_V2 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;

    return result;
}

static inline HM_V2
hm_v2_mul(f32 a, HM_V2 b) {
    HM_V2 result;

    result.x = a * b.x;
    result.y = a * b.y;

    return result;
}

static inline f32
hm_v2_dot(HM_V2 a, HM_V2 b) {
    f32 result = a.x * b.x + a.y * b.y;

    return result;
}

static inline HM_V2
hm_v2_perp(HM_V2 v) {
    HM_V2 result;

    result.x = -v.y;
    result.y = v.x;

    return result;
}

static inline HM_V2
hm_v2_rperp(HM_V2 v) {
    HM_V2 result;

    result.x = v.y;
    result.y = -v.x;

    return result;
}

static inline f32
hm_get_v2_len_sq(HM_V2 v) {
    f32 result = hm_v2_dot(v, v);

    return result;
}

static inline f32
hm_get_v2_len(HM_V2 v) {
    f32 result = sqrtf(hm_get_v2_len_sq(v));

    return result;
}

static inline HM_V2
hm_v2_normalize(HM_V2 v) {
    f32 len = hm_get_v2_len(v);

    HM_V2 result = hm_v2_mul(1.0f / len, v);

    return result;
}

typedef struct {
    i32 exist;
    f32 x;
    f32 y;
} HM_LinearSystem2Solution;

static inline HM_LinearSystem2Solution
hm_solve_linear_system2(HM_V2 a, HM_V2 b, HM_V2 c) {
    HM_LinearSystem2Solution result = {};

    f32 d = a.x * b.y - a.y * b.x;

    if (d != 0.0f) {
        result.exist = 1;
        HM_V2 solution = hm_v2((c.x * b.y - c.y * b.x) / d,
                              (a.x * c.y - a.y * c.x) / d);
        result.x = solution.x;
        result.y = solution.y;
    }

    return result;
}

typedef struct {
    HM_V2 a;
    HM_V2 b;
} HM_Line2;

static inline HM_Line2
hm_line2(HM_V2 a, HM_V2 b) {
    HM_Line2 result;

    result.a = a;
    result.b = b;

    return result;
}

typedef struct {
    HM_V2 o;
    HM_V2 d;
} HM_Ray2;

typedef struct {
    i32 exist;
    f32 t;
} HM_Intersection;

static inline HM_Intersection
hm_ray2_line2_intersection_test(HM_Ray2 ray, HM_Line2 line) {
    HM_Intersection result = {};

    HM_V2 a = ray.d;
    HM_V2 b = hm_v2_sub(line.a, line.b);
    HM_V2 c = hm_v2_sub(line.a, ray.o);

    HM_LinearSystem2Solution solution = hm_solve_linear_system2(a, b, c);
    if (solution.exist &&
        solution.y >= 0 && solution.y <= 1.0 &&
        solution.x >= 0.0)
    {
        result.exist = 1;
        result.t = solution.x;
    }

    return result;
}

static inline HM_Ray2
hm_ray2(HM_V2 o, HM_V2 d) {
    HM_Ray2 result;

    result.o = o;
    result.d = d;

    return result;
}

typedef struct {
    HM_V2 min;
    HM_V2 max;
} HM_BBox2;

static inline HM_BBox2
hm_bbox2(HM_V2 min, HM_V2 max) {
    HM_BBox2 result;

    result.min = min;
    result.max = max;

    return result;
}

static inline HM_BBox2
hm_bbox2_min_size(HM_V2 min, HM_V2 size) {
    HM_BBox2 result;

    result.min = min;
    result.max = hm_v2_add(min, size);

    return result;
}

static inline HM_BBox2
hm_bbox2_cen_size(HM_V2 cen, HM_V2 size) {
    HM_BBox2 result;

    HM_V2 halfsize = hm_v2_mul(0.5f, size);
    result.min = hm_v2_sub(cen, halfsize);
    result.max = hm_v2_add(cen, halfsize);

    return result;
}

static inline HM_V2
hm_get_bbox2_cen(HM_BBox2 rect) {
    // rect.min + 0.5f * (rect.max - rect.min)
    HM_V2 result = hm_v2_add(rect.min, hm_v2_mul(0.5f, hm_v2_sub(rect.max, rect.min)));

    return result;
}

static inline HM_V2
hm_get_bbox2_size(HM_BBox2 rect) {
    HM_V2 result = hm_v2_sub(rect.max, rect.min);

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
} HM_V3;

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

            HM_V3 rgb;
        };

        f32 a;
    };
} HM_V4;

static inline HM_V4
hm_v4(f32 x, f32 y, f32 z, f32 w) {
    HM_V4 result;

    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

#endif
