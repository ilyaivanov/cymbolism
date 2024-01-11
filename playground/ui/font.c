#include <gl/gl.h>
#include "..\..\types.h"

FontData *currentFont;
Layout *currentLayout;
V3f color;

void InitFont(FontData * font, i32 size, char* fontName)
{
    InitFontSystem(font, size, fontName);
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

inline void DrawMyRectGL(float x, float y, i32 w, i32 h)
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
    x = currentLayout->x + x;
    y = currentLayout->y + y;

    for(int i = 0; i < len; i++)
    {
        x += DrawCharBottomLeft(x, y, *(label + i));
    }
}

void SelectFont(FontData* font)
{
    currentFont = font;
}

void SelectLayout(Layout* layout)
{
    currentLayout = layout;
}

void SelectColor(float r, float g, float b)
{
    color.x = r;
    color.y = g;
    color.z = b;
    glColor3f(color.x, color.y, color.z);
}


