#include <gl/gl.h>
#include <stdio.h>
#include <math.h>
#include "math.c"
#include "layout.c"
#include "font.c"
#include "input.c"
#include "item.c"

V3f BG_COLOR = HexColor(0x111111);
#define paddingV PX(20)
#define paddingH PX(16)
#define iconSize PX(7)
#define itemStep PX(20)
#define iconBorderWidth PX(3)
#define iconToText PX(6)
#define scrollWidth PX(8)

typedef struct App
{
    Layout header;
    Layout body;
    Layout footer;
} App;

App app;

void Init()
{
    InitFonts();
}

inline void BuildLayout(V2i screenSize)
{
    f32 headerHeight = PX(50);
    f32 footerHeight = PX(50);
    // f32 leftSidebarWidth = ((sinf(appTime / 500) + 1) * 50);

    f32 bodyWidth = screenSize.x;
    f32 bodyHeight = screenSize.y - (headerHeight + footerHeight);

    app.footer.width = screenSize.x;
    app.footer.height = footerHeight;

    app.body.x = 0;
    app.body.y = app.footer.height;
    app.body.width = bodyWidth; // - ((sinf(appTime / 2500) + 1) * 250);
    app.body.height = bodyHeight;


    app.header.width = screenSize.x;
    app.header.height = headerHeight;
    app.header.y = app.body.y + app.body.height;
}

void DrawItem(Item *item, i32 level)
{
    glColor3f(88.0f / 255.0f, 88.0f / 255.0f, 88.0f / 255.0f);

    float itemX = paddingH + level * itemStep;
    float itemY = app.body.y + app.body.height - app.body.offsetY - app.body.runningY - paddingV;
    DrawSquareCentered(itemX, itemY, iconSize);

    if (item->childrenBuffer.length == 0)
    {
        glColor3f(BG_COLOR.r, BG_COLOR.g, BG_COLOR.g);
        DrawSquareCentered(itemX, itemY, iconSize - iconBorderWidth);
    }

    float textX = itemX + iconSize + iconToText;
    // picked by eye for better font allignment
    float textManualShift = currentFont->textMetric.tmHeight / 15;
    float textY = itemY + textManualShift;
    float width = app.body.width - textX - paddingH - scrollWidth;

    glColor3f(1, 1, 1);

    float paragraphMargin = currentFont->textMetric.tmHeight * 0.2;
    float paragraphHeight = DrawParagraph(item->textBuffer.text, item->textBuffer.length, textX, textY, width) + paragraphMargin;

    if(item==selectedItem)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1, 1, 1, 0.2f);
        DrawRectGLBottomLeft(app.body.x, itemY - paragraphHeight / 2, app.body.width, paragraphHeight);
        glDisable(GL_BLEND);
    }

    app.body.runningY += paragraphHeight;
}
void ChangeSelection(Item *item)
{
    if(item && !IsRoot(item))
        selectedItem = item;
}

void HandleInput(Item *root, UserInput *input)
{
    if(IsMouseOverCurrentLayout(&app.body, input) && app.body.pageHeight > 0 && app.body.pageHeight > app.body.height)
    {
        //pageHeight is calculated on the previous frame, this might be limiting
        app.body.offsetY = Clampf32(app.body.offsetY + input->zDeltaThisFrame, -(app.body.pageHeight - app.body.height), 0);
    }

    if (input->keysPressedhisFrame['J'])
        ChangeSelection(GetItemBelow(selectedItem));
    if (input->keysPressedhisFrame['K'])
        ChangeSelection(GetItemAbove(selectedItem));
    if (input->keysPressedhisFrame['H'] && IsOpen(selectedItem))
        SetIsOpen(selectedItem, 0);
    else if (input->keysPressedhisFrame['H'])
        ChangeSelection(selectedItem->parent);
    if (input->keysPressedhisFrame['L'] && !IsOpen(selectedItem) && ChildCount(selectedItem) > 0)
        SetIsOpen(selectedItem, 1);
    else if (input->keysPressedhisFrame['L'] && ChildCount(selectedItem) > 0)
        ChangeSelection(GetChildAt(selectedItem, 0));

}

void DrawUI(V2i screenSize, Item *root, UserInput *input)
{
    HandleInput(root, input);

    BuildLayout(screenSize);

    glClearColor(BG_COLOR.r, BG_COLOR.g, BG_COLOR.g, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    FillLayout(&app.header, (V3f)HexColor(0x232323));
    FillLayout(&app.footer, (V3f)HexColor(0x444444));


    ClipLayout(&app.body);

    glColor4f(1, 1, 1, 1);

    currentFont = &regularFont;


    float manualFontShift = currentFont->textMetric.tmHeight / 4;

    app.body.runningY = 0;

    ForEachVisibleChild(root, DrawItem);

    app.body.pageHeight = app.body.runningY + paddingV;
    DrawScrollbar(&app.body, scrollWidth);
    EndCliping();
}