#ifndef VEC_H
#define VEC_H

#include "primitives.h"

typedef struct V2i { i32 x, y; } V2i;
typedef struct V3i { i32 x, y, z; } V3i;
typedef struct V2f { f32 x, y; } V2f;
typedef struct V3f { f32 x, y, z; } V3f;



V2f V2fAdd(V2f v1, V2f v2)
{
    return (V2f){v1.x + v2.x, v1.y + v2.y};
}

V2f V2fSub(V2f v1, V2f v2)
{
    return (V2f){v1.x - v2.x, v1.y - v2.y};
}

V2f V2fMulScalar(V2f v1, float scalar)
{
    return (V2f){v1.x * scalar, v1.y * scalar};
}

#endif