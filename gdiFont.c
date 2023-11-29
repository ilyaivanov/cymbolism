#include "types.h"
#include "windows.h"

// char *fontPath = "c:/windows/fonts/cour.ttf";
char *fontPath = "c:/windows/fonts/segoeui.ttf";
// char *fontPath = "c:/windows/fonts/arial.ttf";
// char *fontPath = "c:/windows/fonts/lucon.ttf";
// char *fontPath = "c:/windows/fonts/consola.ttf";

// char *fontName = "Courier New";
char *fontName = "Segoe UI";
// char *fontName = "Consolas";
// char *fontName = "Arial";

//monospaced
// char *fontName = "Lucida Console";

MyBitmap textures[256];

// Need to use ABC structure for this 
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getcharabcwidthsa
u8 widths[256];

TEXTMETRIC textMetric;

int kerningPairCount;
KERNINGPAIR *pairs;

void InitFontSystem(int fontSize)
{
    HDC deviceContext = CreateCompatibleDC(0);
 
    AddFontResourceExA(fontPath, FR_PRIVATE, 0);


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
    int textureSize = 256;
    InitBitmapInfo(&info, textureSize, textureSize);

    void *bits;
    HBITMAP bitmap = CreateDIBSection(deviceContext, &info, DIB_RGB_COLORS, &bits, 0, 0);

    SelectObject(deviceContext, bitmap);
    SelectObject(deviceContext, font);

    
    kerningPairCount = GetKerningPairsA(deviceContext, 0, 0);
    pairs = malloc(sizeof(KERNINGPAIR) * kerningPairCount);
    GetKerningPairsA(deviceContext, kerningPairCount, pairs);

    SetBkColor(deviceContext, RGB(0, 0, 0));
    SetTextColor(deviceContext, RGB(255, 255, 255));

    SIZE size;
    for (char ch = 32; ch <= 126; ch += 1)
    {
        int len = 1;
        GetTextExtentPoint32A(deviceContext, &ch, len, &size);

        widths[ch - 32] = size.cx;
        TextOutA(deviceContext, 0, 0, &ch, len);

        MyBitmap* texture = &textures[ch - 32];
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


    GetTextMetrics(deviceContext, &textMetric);
    DeleteDC(deviceContext);
}

MyBitmap *GetGlyphBitmap(char codepoint)
{
    MyAssert(codepoint >= 32 && codepoint <= 126);
    return &textures[codepoint - 32];
}

u8 GetGlyphWidth(char codepoint)
{
    MyAssert(codepoint >= 32 && codepoint <= 126);
    return widths[codepoint - 32];
}

inline int GetFontHeight()
{
    return textMetric.tmHeight;
}

inline int GetDescent()
{
    return textMetric.tmDescent;
}

inline int GetAscent()
{
    return textMetric.tmAscent;
}


int GetKerningValue(char left, char right)
{
    KERNINGPAIR * pair = pairs;
    // TODO: optimize this into a hash or a nested arrays
    for (int i = 0; i < kerningPairCount; i += 1)
    {
        if (pair->wFirst == left && pair->wSecond == right)
            return pair->iKernAmount;

        pair++;
    }
    return 0;
}