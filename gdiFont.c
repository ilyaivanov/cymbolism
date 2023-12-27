#include "types.h"
#include "windows.h"


inline int HashAndProbeIndex(FontData *font, u16 left, u16 right)
{
    i32 keysMask = ArrayLength(font->pairsHash) - 1;
    i32 index = (left * 19 + right * 7) & keysMask;
    int i = 1;
    while(font->pairsHash[index].left != 0 && font->pairsHash[index].left != left && font->pairsHash[index].right != right)
    {
        index += (i*i);
        i++;
        if(index >= ArrayLength(font->pairsHash))
            index = index & keysMask;
        
    }
    return index;
}

void DrawCharIntoBitmap(MyBitmap *fontCanvas, HDC *deviceContext, MyBitmap* texture, wchar_t ch)
{
    SIZE size;
    GetTextExtentPoint32W(*deviceContext, &ch, 1, &size);

    TextOutW(*deviceContext, 0, 0, &ch, 1);

    texture->width = size.cx;
    texture->height = size.cy;
    texture->bytesPerPixel = 4;

    texture->pixels = AllocateMemory(texture->height * texture->width * texture->bytesPerPixel);

    CopyRectTo(fontCanvas, texture);
}

void InitFontSystem(FontData *fontData, int fontSize, char* fontName)
{
    Start(FontInitialization);

    HDC deviceContext = CreateCompatibleDC(0);
 
 
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
    MyBitmap fontCanvas = {.bytesPerPixel = 4, .height = textureSize, .width = textureSize, .pixels = bits};

    SelectObject(deviceContext, bitmap);
    SelectObject(deviceContext, font);

    Start(FontKerningTablesInitialization);
    i32 kerningPairCount = GetKerningPairsW(deviceContext, 0, 0);
    i32 pairsSizeAllocated = sizeof(KERNINGPAIR) * kerningPairCount;
    KERNINGPAIR *pairs = AllocateMemory(pairsSizeAllocated);
    GetKerningPairsW(deviceContext, kerningPairCount, pairs);

    i32 hashSize = ArrayLength(fontData->pairsHash);
    i32 keysMask = hashSize - 1;
    for(int i = 0; i < kerningPairCount; i++)
    {
        KERNINGPAIR *pair = pairs + i;
        i32 index = HashAndProbeIndex(fontData, pair->wFirst, pair->wSecond);

        fontData->pairsHash[index].left = pair->wFirst;
        fontData->pairsHash[index].right = pair->wSecond;
        fontData->pairsHash[index].val = pair->iKernAmount;
    }
    FreeMemory(pairs);
    Stop(FontKerningTablesInitialization);

    SetBkColor(deviceContext, RGB(0, 0, 0));
    SetTextColor(deviceContext, RGB(255, 255, 255));

    Start(FontTexturesInitialization);

    SIZE size;
    for (wchar_t ch = 32; ch <= MAX_CHAR_CODE; ch += 1)
    {
        int len = 1;
        GetTextExtentPoint32W(deviceContext, &ch, len, &size);

        TextOutW(deviceContext, 0, 0, &ch, len);

        MyBitmap* texture = &fontData->textures[ch];
        texture->width = size.cx;
        texture->height = size.cy;
        texture->bytesPerPixel = 4;

        texture->pixels = AllocateMemory(texture->height * texture->width * texture->bytesPerPixel);

        CopyRectTo(&fontCanvas, texture);
    }


    // BEGIN ugly fucking design for checkmark (sparse unicode handling for 200k symbols will fix this)
    //
    DrawCharIntoBitmap(&fontCanvas, &deviceContext, &fontData->checkmark, 10003); // code for ✓
    DrawCharIntoBitmap(&fontCanvas, &deviceContext, &fontData->chevron, 0x203A); // code for ›
    //
    // END of ugly fucking design for checkmark

    Stop(FontTexturesInitialization);

    GetTextMetrics(deviceContext, &fontData->textMetric);
    DeleteObject(bitmap);
    DeleteDC(deviceContext);

    Stop(FontInitialization);
}

inline MyBitmap *GetGlyphBitmap(FontData *font, char codepoint)
{
    return &font->textures[codepoint];
}

inline u8 GetGlyphWidth(FontData *font,char codepoint)
{
    return font->textures[codepoint].width;
}

inline i32 GetTextWidth(FontData *font, char *text, i32 len)
{
    i32 res = 0;
    for(int i = 0; i < len; i++)
    {
        res += GetGlyphWidth(font, *text) + GetKerningValue(font, *text, *(text + 1));
        text++;
    }
    return res;
}

inline void DrawTextLeftTop(MyBitmap *bitmap, FontData *font, i32 x, i32 y, char *text, i32 len, i32 color)
{
    for (int i = 0; i < len; i += 1)
    {
        char codepoint = *(text + i);

        MyBitmap *glyphBitmap = GetGlyphBitmap(font, codepoint);
        DrawTextureTopLeft(bitmap, glyphBitmap, x, y, color);

        char nextCodepoint = *(text + i + 1);
        
        x += glyphBitmap->width + GetKerningValue(font, codepoint, nextCodepoint);
    }
}

inline void DrawTextLeftBottom(MyBitmap *bitmap, FontData *font, i32 x, i32 y, char *text, i32 len, i32 color)
{
    DrawTextLeftTop(bitmap, font, x, y - font->textMetric.tmHeight, text, len, color);
}

inline void DrawTextCenterBottom(MyBitmap *bitmap, FontData *font, i32 x, i32 y, char *text, i32 len, i32 color)
{
    i32 textWidth = GetTextWidth(font, text, len);
    DrawTextLeftBottom(bitmap, font, x - textWidth / 2, y, text, len, color);
}

inline void DrawTextRightBottom(MyBitmap *bitmap, FontData *font, i32 x, i32 y, char *text, i32 len, i32 color)
{
    i32 textWidth = GetTextWidth(font, text, len);
    DrawTextLeftBottom(bitmap, font, x - textWidth, y, text, len, color);
}

inline void DrawTextLeftCenter(MyBitmap *bitmap, FontData *font, i32 x, i32 y, char *text, i32 len, i32 color)
{
    DrawTextLeftTop(bitmap, font, x, y - font->textMetric.tmHeight / 2, text, len, color);
}

inline int GetKerningValue(FontData *font, u16 left, u16 right)
{
    Start(FramePrintTextFindKerning);
    i32 index = HashAndProbeIndex(font, left, right);

    if(font->pairsHash[index].left != left && font->pairsHash[index].right != right)
    {
        Stop(FramePrintTextFindKerning);
        return 0;
    }

    Stop(FramePrintTextFindKerning);
    return font->pairsHash[index].val;
}