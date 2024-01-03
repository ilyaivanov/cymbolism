#include "types.h"
#include <math.h>

#define BACKGROUND_COLOR_GREY 0x18
#define COLOR_APP_BACKGROUND 0x181818
#define COLOR_MENU_BACKGROUND 0x262626
#define COLOR_SELECTION_BAR_NORMAL_MODE 0x2F2F2F
#define COLOR_SELECTION_BAR_INSERT_MODE 0x2F5F5F

// Typography colors
#define COLOR_SELECTED_ITEM 0xffffff
#define COLOR_NORMAL_ITEM 0xdddddd
#define COLOR_DONE_ITEM 0x999999

#define HEADER_HEIGHT 30
#define FONT_SIZE 14
#define FONT_FAMILY "Segoe UI"
#define ICON_SIZE 10
#define TEXT_TO_ICON 15
#define LEVEL_STEP 40
#define LINE_HEIGHT 1.2f
#define PAGE_PADDING 30
#define BOTTOM_PADDING 50

// Retained vs Immediate

typedef struct Layout
{
    i32 x, y, width, height, runningY;
} Layout;

typedef struct App
{
    Layout header;
    Layout leftSidebar;
    Layout leftBody;
    Layout rightBody;
    Layout footer;
} App;

App app;
Layout *currentLayout;

//
// Changing scroll during draw phase - am I insane?
// Will be removed once somewhat retained UI model will be introduced
// currently the problem is that I don't know what is the position of the element when I change selection
//

inline void ScrollTo(AppState *state, i32 val)
{
    if (state->pageHeight > state->canvas.height)
        state->yOffset = ClampI32(val, 0, state->pageHeight - state->canvas.height);
}

inline void ScrollBy(AppState *state, i32 delta)
{
    ScrollTo(state, state->yOffset + delta);
}

inline void CheckScrollOffset(AppState *state, i32 rectRunningY, i32 rectHeight)
{
    i32 rectAbsolutePos = rectRunningY + state->yOffset;
    i32 rectAbsoluteBottom = rectAbsolutePos + rectHeight;
    if (rectAbsoluteBottom > state->yOffset + state->canvas.height - 60)
        ScrollTo(state, rectAbsoluteBottom - state->canvas.height + 60);
    else if (rectAbsolutePos < state->yOffset + 60)
        ScrollTo(state, rectAbsolutePos - 60);
}

void RenderItem(AppState *state, Item *item, i32 level)
{
    FontData *font = &state->fonts.regular;
    i32 fontHeight = state->fonts.regular.textMetric.tmHeight;

    i32 lineHeightInPixels = fontHeight * LINE_HEIGHT;

    i32 isItemSelected = item == state->selectedItem;
    // if (isItemSelected)
    // {
    //     i32 selectionColor = state->editMode == EditorMode_Insert ? COLOR_SELECTION_BAR_INSERT_MODE : COLOR_SELECTION_BAR_NORMAL_MODE;
    //     i32 rectY = currentLayout->runningY - lineHeightInPixels / 2;
    //     i32 rectHeight = (item->newLinesCount + (LINE_HEIGHT - 1)) * fontHeight;

    //     if (state->pageHeight > state->canvas.height)
    //         CheckScrollOffset(state, rectY, rectHeight);

    //     DrawRect(&state->canvas, 0, rectY, state->canvas.width, rectHeight, selectionColor);
    // }

    i32 itemX = currentLayout->x + level * LEVEL_STEP;
    i32 itemY = currentLayout->runningY;
    DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE, 0x888888);

    if (ChildCount(item) == 0)
        DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE - 4, COLOR_APP_BACKGROUND);

    i32 isDone = IsDone(item);
    i32 textColor = isDone ? COLOR_DONE_ITEM : isItemSelected ? COLOR_SELECTED_ITEM
                                                              : COLOR_NORMAL_ITEM;

    if (isDone)
    {
        MyBitmap *t = &state->fonts.regular.checkmark;
        DrawTextureCentered(&state->canvas, t, state->canvas.width - PAGE_PADDING, itemY, textColor);
    }

    i32 textX = itemX + ICON_SIZE / 2 + TEXT_TO_ICON;
    i32 maxWidth = currentLayout->width - (textX - currentLayout->x) - (ICON_SIZE / 2 + TEXT_TO_ICON);
    SplitTextIntoLines(item, &state->fonts.regular, maxWidth);
    Start(FramePrintText);

    for (int i = 1; i <= item->newLinesCount; i++)
    {
        // fontVerticalOffset is picked by hand judging purely by eye, without this text seems off
        i32 fontVerticalOffset = FONT_SIZE / 10 - 1;
        i32 textY = currentLayout->runningY - fontVerticalOffset;
        i32 start = (item->newLines[i - 1] == 0 ? item->newLines[i - 1] : item->newLines[i - 1] + 1);
        char *text = item->textBuffer.text + start;
        i32 lineLength = item->newLines[i] - start;

        Start(FramePrintTextDrawTexture);
        DrawTextLeftCenter(&state->canvas, font, textX, textY, text, lineLength, textColor);
        Stop(FramePrintTextDrawTexture);

        if ((state->isCursorVisible || state->editMode == EditorMode_Insert) && item == state->selectedItem && state->cursorPos >= start && state->cursorPos <= item->newLines[i])
        {
            i32 cursorPosOnLine = state->cursorPos - start;
            DrawRect(&state->canvas, textX + GetTextWidth(font, text, cursorPosOnLine), textY - fontHeight / 2, 1, fontHeight, 0xffffff);
        }
        currentLayout->runningY += fontHeight;
    }
    currentLayout->runningY += fontHeight * (LINE_HEIGHT - 1);
    Stop(FramePrintText);
}

void DrawApp2(AppState *state)
{
    i32 lineHeightInPixels = state->fonts.regular.textMetric.tmHeight * LINE_HEIGHT;

    i32 hasHeader = state->focusedItem != &state->root;

    // state->runningX = PAGE_PADDING;
    // state->runningY = (hasHeader ? HEADER_HEIGHT : PAGE_PADDING) + lineHeightInPixels / 2 - state->yOffset;

    ForEachVisibleChild(state, state->focusedItem, RenderItem);

    // if (hasHeader)
    // {
    //     char *txt = state->focusedItem->textBuffer.text;
    //     i32 len   = state->focusedItem->textBuffer.length;
    //     DrawTextLeftCenter(&state->canvas, &state->fonts.regular, 8, HEADER_HEIGHT / 2, txt, len, COLOR_DONE_ITEM);
    // }

    // if(state->pageHeight > state->canvas.height)
    // {
    //     i32 scrollY = (i32)((float)state->yOffset * (float)(state->canvas.height / (float)state->pageHeight));
    //     i32 scrollHeight = state->canvas.height * state->canvas.height / state->pageHeight;
    //     i32 scrollWidth = 15;
    //     DrawRect(&state->canvas, state->canvas.width - scrollWidth, scrollY, scrollWidth, scrollHeight, 0x552D2E);
    // }

    // // Drawing status bar at the bottom
    // FontData *font = &state->fonts.regular;
    // i32 labelsC = 0x888888;
    // i32 padding = 10;

    // i32 r = font->textMetric.tmHeight + padding * 1.3;
    // DrawRect(&state->canvas, 0, state->canvas.height - r, state->canvas.width, r, COLOR_MENU_BACKGROUND);

    // char *label = state->editMode == EditorMode_Normal ? "Normal" : "Insert";
    // DrawTextLeftBottom(&state->canvas, font, padding, state->canvas.height - padding, label, strlen(label), labelsC);

    // char *label2 = state->isFileSaved ? "Saved" : "Modified";
    // DrawTextCenterBottom(&state->canvas, font, state->canvas.width / 2, state->canvas.height - padding, label2, strlen(label2), labelsC);
}

i32 lastWidth = 0;
float time = 0;
inline void ResizeUI(AppState *state)
{
    i32 headerHeight = 50;
    i32 footerHeight = 50;

    i32 leftSidebarWidth = (i32)(roundf((sinf(time / 500) + 1) * 50));
    i32 bodyWidth = (state->canvas.width - leftSidebarWidth) / 2;
    i32 bodyHeight = state->canvas.height - (headerHeight + footerHeight);

    app.header.width = state->canvas.width;
    app.header.height = headerHeight;

    app.leftSidebar.width = leftSidebarWidth;
    app.leftSidebar.y = app.header.height;
    app.leftSidebar.height = bodyHeight;

    app.leftBody.x = app.leftSidebar.width;
    app.leftBody.y = app.header.height;
    app.leftBody.width = bodyWidth;
    app.leftBody.height = bodyHeight;

    app.rightBody.x = app.leftSidebar.width + bodyWidth;
    app.rightBody.y = app.header.height;
    app.rightBody.width = bodyWidth;
    app.rightBody.height = app.leftBody.height;

    app.footer.y = app.leftBody.y + app.leftBody.height;
    app.footer.width = state->canvas.width;
    app.footer.height = footerHeight;
}

i32 IsMouseOver(Layout *layout, MyInput *input)
{
    return layout->x <= input->mouseX && (layout->x + layout->width) > input->mouseX &&
           layout->y <= input->mouseY && (layout->y + layout->height) > input->mouseY;
}

void Fill(AppState *state, i32 color)
{
    DrawRect(&state->canvas, currentLayout->x, currentLayout->y, currentLayout->width, currentLayout->height, color);
}

void DrawApp(AppState *state, MyInput *input)
{

    time += 16.666f;
    ResizeUI(state);

    currentLayout = &app.header;
    Fill(state, 0x223333);

    currentLayout = &app.leftSidebar;
    Fill(state, 0x333333);

    currentLayout = &app.leftBody;
    Fill(state, 0x222222);
    currentLayout->runningY = currentLayout->y;
    ForEachVisibleChild(state, state->focusedItem, RenderItem);

    currentLayout = &app.rightBody;
    Fill(state, 0x444444);
    currentLayout->runningY = currentLayout->y;
    ForEachVisibleChild(state, state->focusedItem, RenderItem);

    currentLayout = &app.footer;
    Fill(state, 0x555555);

    char buff[256];
    sprintf(buff, "x:%d y:%d", input->mouseX, input->mouseY);
    DrawTextLeftBottom(&state->canvas, &state->fonts.regular, 4, state->canvas.height - 4, buff, strlen(buff), 0xffffff);
}