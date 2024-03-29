#ifndef MATH_C
#define MATH_C

#include <stdint.h>

#define ArrayLength(array) (sizeof(array) / sizeof(array[0]))
#define Assert(cond) if (!(cond)) { *(u32*)0 = 0; }
#define Fail(msg) Assert(0)

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;



typedef struct V2i { i32 x, y; } V2i;
typedef struct V3i { i32 x, y, z; } V3i;
typedef struct V2f { f32 x, y; } V2f;
typedef struct V3f { 
    union { 
        struct { f32 x, y, z; };
        struct { f32 r, g, b; };
    };
}  V3f;



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

#define HexColor(hex) {(float)((hex >> 16) & 0xff) / 255.0f, \
                            (float)((hex >> 8) & 0xff) / 255.0f,  \
                            (float)((hex >> 0) & 0xff) / 255.0f}


inline i32 Clampi32(i32 val, i32 min, i32 max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}


inline f32 Clampf32(f32 val, f32 min, f32 max)
{
    if (val < min)
        return min;
    if (val > max)
        return max;
    return val;
}

#endif