#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define ArrayLength(array) (sizeof(array) / sizeof(array[0]))
#define MyAssert(cond) if (!(cond)) { *(u32*)0 = 0; }

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

typedef struct V2f { float x, y; } V2f;
typedef struct V3f { float x, y, z; } V3f;
typedef struct Color { float r, g, b, a; } Color;

typedef struct MyBitmap
{
    i32 width;
    i32 height;
    i32 bytesPerPixel;
    u32 *pixels;
} MyBitmap;

typedef struct StringBuffer
{
    char *text;
    i32 capacity;
    i32 length;
} StringBuffer;

typedef struct MyInput
{
    i32 keysPressed[256];
    i32 isPressed[256];
} MyInput;

typedef struct FontData {
    MyBitmap textures[256];

    // Need to use ABC structure for this 
    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getcharabcwidthsa
    u8 widths[256];

    TEXTMETRIC textMetric;

    int kerningPairCount;
    KERNINGPAIR *pairs;
} FontData;

typedef struct Fonts
{
    FontData regular;
} Fonts;

typedef enum EditMode {
    EditMode_Normal,
    EditMode_Edit,
} EditMode;

typedef struct Player
{
    V2f center;
    float positionRadians;
} Player;

typedef struct RotatingEnemy
{
    V2f center;
    float positionRadians;
    float rotationSpeed;
} RotatingEnemy;

typedef struct AppState {
    Fonts fonts;
    MyBitmap canvas;
    float degrees;
    RotatingEnemy enemies[32];
    i32 enemiesCount;
    i32 circlesComplete;
} AppState;

#endif