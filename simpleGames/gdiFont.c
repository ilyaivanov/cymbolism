#include "types.h"
#include "windows.h"

void InitFontSystem(FontData *fontData, int fontSize, char* fontName)
{
    HDC deviceContext = CreateCompatibleDC(0);
 
    // AddFontResourceExA(fontPath, FR_PRIVATE, 0);

    int h = -MulDiv(fontSize, GetDeviceCaps(deviceContext, LOGPIXELSY), 96); // WTF is 96 or 72
    HFONT font = CreateFontA(h, 0, 0, 0,
                             FW_DONTCARE, // Weight
                             0,           // Italic
                             0,           // Underline
                             0,           // Strikeout
                             DEFAULT_CHARSET,
                             OUT_TT_ONLY_PRECIS,
                             CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY,
                             DEFAULT_PITCH,
                             fontName);

    BITMAPINFO info = {0};
    int textureSize = 1024;
    InitBitmapInfo(&info, textureSize, textureSize);

    void *bits;
    HBITMAP bitmap = CreateDIBSection(deviceContext, &info, DIB_RGB_COLORS, &bits, 0, 0);

    SelectObject(deviceContext, bitmap);
    SelectObject(deviceContext, font);

    
    fontData->kerningPairCount = GetKerningPairsA(deviceContext, 0, 0);
    fontData->pairs = malloc(sizeof(KERNINGPAIR) * fontData->kerningPairCount);
    GetKerningPairsA(deviceContext, fontData->kerningPairCount, fontData->pairs);

    SetBkColor(deviceContext, RGB(0, 0, 0));
    SetTextColor(deviceContext, RGB(255, 255, 255));

    SIZE size;
    for (char ch = 32; ch <= 126; ch += 1)
    {
        int len = 1;
        GetTextExtentPoint32A(deviceContext, &ch, len, &size);

        fontData->widths[ch - 32] = size.cx;
        TextOutA(deviceContext, 0, 0, &ch, len);

        MyBitmap* texture = &fontData->textures[ch - 32];
        texture->width = size.cx;
        texture->height = size.cy;
        texture->bytesPerPixel = 4;

        texture->pixels = malloc(texture->height * texture->width * texture->bytesPerPixel);

        u32 *row = (u32 *)texture->pixels;
        u32 *source = (u32 *)bits;
        for (u32 y = 0; y < texture->height; y += 1)
        {
            u32 *pixel = row;
            u32 *sourcePixel = source;
            for (u32 x = 0; x < texture->width; x += 1)
            {
                *pixel = *sourcePixel;
                sourcePixel += 1;
                pixel += 1;
            }
            source += textureSize;
            row += texture->width;
        }
    }


    GetTextMetrics(deviceContext, &fontData->textMetric);
    DeleteDC(deviceContext);
}

MyBitmap *GetGlyphBitmap(FontData *font, char codepoint)
{
    MyAssert(codepoint >= 32 && codepoint <= 126);
    return &font->textures[codepoint - 32];
}

u8 GetGlyphWidth(FontData *font,char codepoint)
{
    MyAssert(codepoint >= 32 && codepoint <= 126);
    return font->widths[codepoint - 32];
}

inline int GetFontHeight(FontData *font)
{
    return font->textMetric.tmHeight;
}

inline int GetDescent(FontData *font)
{
    return font->textMetric.tmDescent;
}

inline int GetAscent(FontData *font)
{
    return font->textMetric.tmAscent;
}

inline i32 GetTextWidth(FontData *font, char *text, i32 len){
    i32 res = 0;
    for(int i = 0; i < len; i++)
    {
        res += GetGlyphBitmap(font, *text)->width + GetKerningValue(font, *text, *(text + 1));
        text++;
    }
    return res;
}

inline void DrawTextLeftCentered(MyBitmap *bitmap, FontData *font, i32 x, i32 y, char *text, i32 color)
{
    int len = strlen(text);
    i32 topLine = y - font->textMetric.tmHeight / 2;
    
    for (int i = 0; i < len; i += 1)
    {
        char codepoint = *text;
        MyBitmap *glyphBitmap = GetGlyphBitmap(font, codepoint);
        DrawTextureTopLeft(bitmap, glyphBitmap, x, topLine, color);

        char nextCodepoint = *(text + 1);
        x += glyphBitmap->width + GetKerningValue(font, codepoint, nextCodepoint);
        text++;
    }

}

inline void DrawTextLeftBottom(MyBitmap *bitmap, FontData *font, i32 x, i32 y, char *text, i32 color){
    DrawTextLeftCentered(bitmap, font, x, y - font->textMetric.tmHeight / 2, text, color);
}

int GetKerningValue(FontData *font, char left, char right)
{
    KERNINGPAIR * pair = font->pairs;
    // TODO: optimize this into a hash or a nested arrays
    for (int i = 0; i < font->kerningPairCount; i += 1)
    {
        if (pair->wFirst == left && pair->wSecond == right)
            return pair->iKernAmount;

        pair++;
    }
    return 0;
}