#include <gl/gl.h>
#include "..\..\types.h"
#include "..\..\vec.c"
#include "font.c"


void OnSizeChange(V2i *size)
{
    glViewport(0, 0, size->x, size->y);

    // allows me to set vecrtex coords as 0..width/height, instead of -1..+1
    // 0,0 is bottom left, not top left
    // matrix in code != matrix in math notation, details at https://youtu.be/kBuaCqaCYwE?t=3084
    // in short: rows in math are columns in code
    float w = 2.0f / size->x;
    float h = 2.0f / size->y;
    GLfloat m[] = {
         w,  0,  0,  0,   
         0,  h,  0,  0,   
         0,  0,  1,  0,   
        -1, -1,  0,  1,   
    };
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m);
}

inline u8 IsKeyPressed(u8 *keyboardState, u8 key)
{
    return keyboardState[key] & 0b10000000;
}

inline u32 IsMouseOverLayout(Layout *layout, V2i * mouse)
{
    return layout->x < mouse->x && layout->x + layout->width > mouse->x &&
           layout->y < mouse->y && layout->y + layout->height > mouse->y;
}

GLuint cachedTextures[MAX_CHAR_CODE];


App rootLayout;

inline void ReflowLayout(V2i *screenSize)
{
    f32 headerHeight = 50;
    f32 footerHeight = 50;
    // f32 leftSidebarWidth = ((sinf(appTime / 500) + 1) * 50);

    f32 bodyWidth = screenSize->x;
    f32 bodyHeight = screenSize->y - (headerHeight + footerHeight);

    rootLayout.footer.width = screenSize->x;
    rootLayout.footer.height = footerHeight;

    rootLayout.body.x = 0;
    rootLayout.body.y = rootLayout.header.height;
    rootLayout.body.width = bodyWidth;
    rootLayout.body.height = bodyHeight;


    rootLayout.header.width = screenSize->x;
    rootLayout.header.height = headerHeight;
    rootLayout.header.y = rootLayout.body.y + rootLayout.body.height;
}

void InitUI(AppState *app, V2i *screenSize)
{
    InitFont(&app->fonts.regular, 14, "Segoe UI");
    InitFont(&app->fonts.big, 40,  "Segoe UI Semibold");
}

inline void FillLayout(Layout* layout)
{
    DrawMyRectGL(layout->x, layout->y, layout->width, layout->height);
}

void DrawUI(AppState *app, V2i *screenSize, u8 *keyboardState, V2i *mouse, i32* zDelta)
{
    ReflowLayout(screenSize);

    glClearColor(11.0f / 255, 11.0f / 255, 11.0f / 255, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    Layout * layout = &rootLayout.body;
    if(IsMouseOverLayout(layout, mouse))
    {
        glColor3f(0.13f, 0.13f, 0.13f);
        //TODO: layout->pageHeight is computed from the previous frame
        layout->offsetY = ClampI32(layout->offsetY + *zDelta, -layout->pageHeight + layout->height, 0);
    }
    else 
        glColor3f(0.1f, 0.1f, 0.1f);

    FillLayout(layout);

    glColor3f(0.0f, 0.3f, 0.0f);
    FillLayout(&rootLayout.footer);

    glColor3f(0.3f, 0.0f, 0.0f);
    FillLayout(&rootLayout.header);
  
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    SelectLayout(layout);

    glEnable(GL_SCISSOR_TEST);
    glScissor(layout->x, layout->y, layout->width, layout->height);
    SelectColor(1, 1, 1);

    float startY = layout->height - layout->offsetY;

    glEnable(GL_TEXTURE_2D);
    SelectFont(&app->fonts.big);
    float runningY = startY - app->fonts.big.textMetric.tmHeight;
    char * title = "Title";
    DrawLabel(title, strlen(title), 20, runningY);
    
    SelectFont(&app->fonts.regular);

    u8 buff[256];
    Item dummyItem = {0};
    dummyItem.textBuffer.text = buff;
    Item * item = &dummyItem;
    float x = 20;
    for(int i = 0; i < 100; i++)
    {
        dummyItem.textBuffer.length = sprintf(buff, "%d. Word and Stuff and all of the other things to do in the world with the stuff you know what? well. something has to do ", i + 1);
        SplitTextIntoLines(&dummyItem, currentFont, layout->width - x - 20);
        for(int l = 1; l <= dummyItem.newLinesCount; l++)
        {
            i32 start = (item->newLines[l - 1] == 0 ? item->newLines[l - 1] : item->newLines[l - 1] + 1);
            char *text = item->textBuffer.text + start;
            i32 lineLength = item->newLines[l] - start;

            runningY -= app->fonts.regular.textMetric.tmHeight;
            DrawLabel(text, lineLength, x, runningY);
        }
    }
    glDisable(GL_TEXTURE_2D);


    layout->pageHeight = -runningY + startY;
    if(layout->pageHeight > layout->height)
    {
        i32 scrollY = layout->offsetY * (layout->height / layout->pageHeight);
        i32 scrollHeight = layout->height * (layout->height / layout->pageHeight);
        i32 scrollWidth = 15;
        glColor4f(1, 1, 1, 0.3f);
        // SelectColor(0.3f, 0.3f, 0.3f);
        DrawMyRectGL(layout->x + layout->width - scrollWidth, 
                     layout->y + layout->height - scrollHeight + scrollY, 
                     scrollWidth,
                     scrollHeight);
    }

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);

}