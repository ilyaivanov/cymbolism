#ifndef FONT_C
#define FONT_C

#include <windows.h>
#include <gl/gl.h>
#include "math.c"
#include "memory.c"

// this dependency is questionable
#include "layout.c"

#define MAX_CHAR_CODE 200

typedef struct MyBitmap
{
    i32 width;
    i32 height;
    i32 bytesPerPixel;
    u32 *pixels;
} MyBitmap;

typedef struct FontKerningPair
{
    u16 left;
    u16 right;
    i8 val;
} FontKerningPair;

typedef struct FontData 
{
    MyBitmap textures[MAX_CHAR_CODE];
    GLuint cachedTextures[MAX_CHAR_CODE];

    // stupid fucking design, but I need to create sparse system for 200k unicode chars
    MyBitmap checkmark;

    // Need to use ABC structure for this 
    // https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getcharabcwidthsa

    TEXTMETRIC textMetric;

    FontKerningPair pairsHash[16 * 1024]; // Segoe UI has around 8k pairs
} FontData;

FontData titleFont;
FontData regularFont;
FontData smallFont;


FontData *currentFont;

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


// takes dimensions of destinations, reads rect from source at (0,0)
inline void CopyRectTo(MyBitmap *sourceT, MyBitmap *destination)
{
    u32 *row = (u32 *)destination->pixels + destination->width * (destination->height - 1);
    u32 *source = (u32 *)sourceT->pixels + sourceT->width * (sourceT->height - 1);
    for (u32 y = 0; y < destination->height; y += 1)
    {
        u32 *pixel = row;
        u32 *sourcePixel = source;
        for (u32 x = 0; x < destination->width; x += 1)
        {
            *pixel = *sourcePixel;
            sourcePixel += 1;
            pixel += 1;
        }
        source -= sourceT->width;
        row -= destination->width;
    }
}

void InitFontSystem(FontData *fontData, int fontSize, char* fontName)
{

    HDC deviceContext = CreateCompatibleDC(0);
 
 
    int h = -MulDiv(fontSize, GetDeviceCaps(deviceContext, LOGPIXELSY), USER_DEFAULT_SCREEN_DPI);
    HFONT font = CreateFontA(h, 0, 0, 0,
                             FW_DONTCARE, // Weight
                             0,           // Italic
                             0,           // Underline
                             0,           // Strikeout
                             DEFAULT_CHARSET,
                             OUT_TT_ONLY_PRECIS,
                             CLIP_DEFAULT_PRECIS,

                             //I've experimented with the Chrome and it doesn't render LCD quality for fonts above 32px
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

    SetBkColor(deviceContext, RGB(0, 0, 0));
    SetTextColor(deviceContext, RGB(255, 255, 255));


    SIZE size;
    for (wchar_t ch = 32; ch < MAX_CHAR_CODE; ch += 1)
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
    // DrawCharIntoBitmap(&fontCanvas, &deviceContext, &fontData->checkmark, 10003); // code for ✓
    // DrawCharIntoBitmap(&fontCanvas, &deviceContext, &fontData->chevron, 0x203A); // code for ›
    // DrawCharIntoBitmap(&fontCanvas, &deviceContext, &fontData->c1, '💉'); // code for 💉
    // DrawCharIntoBitmap(&fontCanvas, &deviceContext, &fontData->c2, '🚑'); // code for 🚑
    //
    // END of ugly fucking design for checkmark


    GetTextMetrics(deviceContext, &fontData->textMetric);
    DeleteObject(bitmap);
    DeleteDC(deviceContext);

}

void CreateFontTexturesForOpenGl(FontData *font)
{
    glGenTextures(MAX_CHAR_CODE, font->cachedTextures);
    for (u32 i = ' '; i < MAX_CHAR_CODE; i++)
    {
        MyBitmap *bit = &font->textures[i];
        glBindTexture(GL_TEXTURE_2D, font->cachedTextures[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, bit->width, bit->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, bit->pixels);
    }
}

void InitFonts()
{
    InitFontSystem(&titleFont, 40, "Segoe UI Bold");
    CreateFontTexturesForOpenGl(&titleFont);

    InitFontSystem(&regularFont, 14, "Segoe UI");
    CreateFontTexturesForOpenGl(&regularFont);
    
    InitFontSystem(&smallFont, 12, "Segoe UI");
    CreateFontTexturesForOpenGl(&smallFont);
}

inline void DrawRectGLBottomLeft(float x, float y, i32 w, i32 h)
{
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2f(x, y + h);

    glVertex2f(x, y);

    glVertex2f(x + w, y + h);

    glVertex2f(x + w, y);
    glEnd();
}

u32 DrawCharBottomLeft(float x, float y, u32 ch)
{
    if (ch > MAX_CHAR_CODE)
        return 0;

    MyBitmap *bitmap = &currentFont->textures[ch];

    glBindTexture(GL_TEXTURE_2D, currentFont->cachedTextures[ch]);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 1);
    glVertex2f(x, y + bitmap->height);

    glTexCoord2f(0, 0);
    glVertex2f(x, y);

    glTexCoord2f(1, 1);
    glVertex2f(x + bitmap->width, y + bitmap->height);

    glTexCoord2f(1, 0);
    glVertex2f(x + bitmap->width, y);
    glEnd();
    return bitmap->width;
}


void DrawLabel(char* label, i32 len, float x, float y)
{
    for(int i = 0; i < len; i++)
    {
        u8 ch = *(label + i);
        // skips new lines and other control symbols
        if(ch >= ' ')
            x += DrawCharBottomLeft(x, y, ch) + (GetKerningValue(ch, *(label + i + 1)));
    }
}

void SplitTextIntoLines(u8 *text, i32 len,  u32 maxWidth, u32 *newLines, u32 *newLinesCount)
{
    int currentLine = 0;
    int runningWidth = 0;
    int wordFirstLetterIndex = 0;
    int wordLength = 0;

    for (int i = 0; i < len; i += 1)
    {
        i8 ch = *(text + i);
        if(ch < ' ')
            continue;

        if ((ch == ' ' || i == len - 1) && runningWidth >= maxWidth)
        {
            *(newLines + currentLine) = wordFirstLetterIndex - 1;
            currentLine++;
            runningWidth = wordLength;
            wordFirstLetterIndex = i + 1;
            wordLength = 0;
        }
        else
        {
            int w = currentFont->textures[ch].width + GetKerningValue(ch, *(text + i + 1));
            if (ch == ' ')
            {
                wordFirstLetterIndex = i + 1;
                wordLength = 0;
            }
            else
            {
                wordLength += w;
            }
            runningWidth += w;
        }
    }

    *(newLines + currentLine) = len;
    currentLine++;
    *newLinesCount = currentLine;
}

// first line is centered at y, next lines go down
u32 DrawParagraph(char* label, i32 len, float x, float y, float width, u32 *newLines, u32 *newLinesCount)
{
    SplitTextIntoLines(label, len, width, newLines, newLinesCount);


    float paragraphHeight = currentFont->textMetric.tmHeight * *newLinesCount;

    y -= currentFont->textMetric.tmHeight / 2;

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    for(int i = 0; i < *newLinesCount; i++)
    {
        i32 start = (i == 0 ? 0 : newLines[i - 1] + 1);
        char *text = label + start;
        i32 lineLength = newLines[i] - start;

        DrawLabel(text, lineLength, x, y);
        y -= currentFont->textMetric.tmHeight;
    }
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    return paragraphHeight;
}

i32 GetTextWidth(u8 *text, u32 len){
    i32 res = 0;
    for(int i = 0; i < len; i++)
    {
        u8 ch = *(text + i);
        // skips new lines and other control symbols
        if(ch >= ' ')
            res += currentFont->textures[ch].width + (GetKerningValue(ch, *(text + i + 1)));
    }
    return res;
}

inline int GetKerningValue(u16 left, u16 right)
{
    i32 index = HashAndProbeIndex(currentFont, left, right);

    if(currentFont->pairsHash[index].left != left && currentFont->pairsHash[index].right != right)
    {
        return 0;
    }

    return currentFont->pairsHash[index].val;
}

#endif