#include "windows.h"
#include "types.h"
#include "drawing.c"
#include "memory.c"
#include "win_utils.c"
#include "performance.c"
#include "gdiFont.c"

// #define MY_COLOR 0xffffff
#define MY_COLOR 0xff0000

__declspec(dllexport) 
void UpdateAndDraw(MyBitmap *canvas, AppState *state)
{
    DrawRect(canvas, 20, 20, 200, 200, 0xff0000);
}