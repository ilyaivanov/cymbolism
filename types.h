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


typedef struct FileContent
{
    char *content;
    i32 size;
} FileContent;


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

typedef struct Item
{
    struct Item *parent;
    struct Item *children;
    i32 childrenCount;
    i32 isOpen;
    StringBuffer textBuffer;

    //This is a UI feature, I don't need to mix domain entities and UI representation
    u32 newLines[64];
    u32 newLinesCount;
} Item;

typedef struct ItemInStack
{
    Item *ref;
    int level;
} ItemInStack;

typedef struct MyInput
{
    i32 keysPressed[256];
    WPARAM charEventsThisFrame[32];
    i32 charEventsThisFrameCount;
    i32 isPressed[256];
} MyInput;

typedef struct FontData 
{
    MyBitmap textures[2000];

    // Need to use ABC structure for this 
    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getcharabcwidthsa
    u8 widths[2000];

    TEXTMETRIC textMetric;

    int kerningPairCount;
    KERNINGPAIR *pairs;
} FontData;

typedef struct Fonts
{
    FontData regular;
} Fonts;

typedef enum EditorMode
{
    EditorMode_Normal,
    EditorMode_Insert,
} EditorMode;

typedef struct AppState 
{
    Item root;
    Item *selectedItem;
    Fonts fonts;
    MyBitmap canvas;
    EditorMode editMode;

    i32 cursorPos;
    i32 isFileSaved;
} AppState;

#endif