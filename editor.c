#include <stdio.h>
#include "types.h"
#include "text.c"

#define BACKGROUND_COLOR_GREY 0x11
#define FONT_SIZE 60
#define FONT_FAMILY "Segoe UI"

Item root = {0};
void InitApp(AppState *state)
{
    InitFontSystem(&state->fonts.regular, FONT_SIZE, FONT_FAMILY);
}



void UpdateAndDrawApp(AppState *state, MyInput *input)
{
    u16 bytes[] = { 0x0041, 0x0057, 0x0041, 0x0059, 0x0020, 0x0041, 0x0056, 0x003f, 0x0020, 0x0417, 0x0432, 0x0456, 0x0441, 0x043d, 0x043e, 0x0021  };
    u32 len =  ArrayLength(bytes);

    i32 x = 20;

    u32 currentCodepoint = 0;
    u32 prevCodepoint = 0;

    for(int i = 0; i < len; i++)
    {
        currentCodepoint = bytes[i];
        
        x += GetKerningValue(&state->fonts.regular, prevCodepoint, currentCodepoint);

        DrawTextureTopLeft(&state->canvas, &state->fonts.regular.textures[currentCodepoint], x, 20, 0xffffff);
        x += state->fonts.regular.widths[currentCodepoint];

        prevCodepoint = currentCodepoint;
    }
}
