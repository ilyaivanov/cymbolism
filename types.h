#ifndef TYPES_H
#define TYPES_H

#include "primitives.h"

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

typedef struct ItemChildrenBuffer
{
    struct Item **children;
    i32 capacity;
    i32 length;
} ItemChildrenBuffer;

typedef enum ItemStateFlag
{
    ItemStateFlag_IsOpen,
    ItemStateFlag_IsDone,

    // indicated wheather I read this item as "[ ] My item" or "[y] My item" from a file
    // I don't want to obtruce the file by always placings [ ] for each item, only if any of the flags are set or it was like that in the file
    ItemStateFlag_IsSerializedWithPlaceholder,
} ItemStateFlag;

typedef struct Item
{
    struct Item *parent;
    ItemChildrenBuffer childrenBuffer;
    StringBuffer textBuffer;

    u32 flags; 

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
    i32 wheelDelta;
    i32 mouseX, mouseY;
} MyInput;

typedef struct FontKerningPair
{
    u16 left;
    u16 right;
    i8 val;
} FontKerningPair;

#define MAX_CHAR_CODE 200

typedef struct FontData 
{
    MyBitmap textures[MAX_CHAR_CODE];

    // stupid fucking design, but I need to create sparse system for 200k unicode chars
    MyBitmap checkmark;
    MyBitmap chevron;

    // Need to use ABC structure for this 
    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getcharabcwidthsa

    TEXTMETRIC textMetric;

    FontKerningPair pairsHash[16 * 1024]; // Segoe UI has around 8k pairs
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
    Item *focusedItem;
    Fonts fonts;

    // used to detect screen width changes in the render function without OnResize event
    u32 lastWidthRenderedWith;

    MyBitmap canvas;
    EditorMode editMode;

    i32 yOffset;
    i32 cursorPos;
    i32 isCursorVisible;
    i32 isFileSaved;

    // TODO: this will be moved in to the model. Currently I'm relying on the height from the prev frame
    i32 pageHeight;

    //used when rendering items as "closures"
    i32 runningX;
    i32 runningY;
} AppState;

#endif