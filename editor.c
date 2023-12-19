#include <stdio.h>
#include "types.h"
#include "text.c"
#include "number.c"
#include "item.c"
#include "cursorAndSelection.c"

#define BACKGROUND_COLOR_GREY 0x18
#define COLOR_APP_BACKGROUND 0x181818
#define COLOR_SELECTION_BAR 0x2F2F2F
#define COLOR_SELECTED_ITEM 0xffffff
#define COLOR_NORMAL_ITEM   0xdddddd

#define FONT_SIZE 14
#define FONT_FAMILY "Segoe UI"
#define ICON_SIZE 10
#define TEXT_TO_ICON 15
#define LEVEL_STEP 40
#define LINE_HEIGHT 1.2f
#define PAGE_PADDING 30


void SplitTextIntoLines(Item *item, FontData *font, u32 maxWidth)
{
    int currentLine = 0;
    int runningWidth = 0;
    int wordFirstLetterIndex = 0;
    int wordLength = 0;

    for (int i = 0; i < item->textBuffer.length; i += 1)
    {
        i8 ch = *(item->textBuffer.text + i);
        
        if ((ch == ' ' || i == item->textBuffer.length - 1) && runningWidth >= maxWidth)
        {
            item->newLines[++currentLine] = wordFirstLetterIndex - 1;        
            runningWidth = wordLength;
            wordFirstLetterIndex = i + 1;
            wordLength = 0;
        }
        else 
        {
            int w = GetGlyphWidth(font, ch);
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

    item->newLines[++currentLine] = item->textBuffer.length;   
    item->newLinesCount = currentLine;
}


void InitApp(AppState *state)
{
    Start(StartUp);
    
    InitFontSystem(&state->fonts.regular, FONT_SIZE, FONT_FAMILY);
    InitRoot(&state->root);
    state->selectedItem = state->root.children;

    Stop(StartUp);

    PrintStartupResults();
}

typedef void ForEachHandler(AppState *state, Item * item, i32 level);

//TODO: check macros if function call would be too slow
//This will be moved inside UI model
inline void ForEachVisibleChild(AppState *state, Item *parent, ForEachHandler *handler)
{
    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = parent->childrenCount - 1; c >= 0; c--)
        stack[++currentItemInStack] = (ItemInStack){parent->children + c, 0};

    while (currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        handler(state, current.ref, current.level);

        if (item->isOpen)
        {
            for (int c = item->childrenCount - 1; c >= 0; c--)
                stack[++currentItemInStack] = (ItemInStack){item->children + c, current.level + 1};
        }
    }
}

void UpdateLines(AppState *state, Item *item, i32 level)
{
    i32 maxWidth = state->canvas.width - PAGE_PADDING * 2 - (ICON_SIZE / 2 + TEXT_TO_ICON) - level * LEVEL_STEP;
    SplitTextIntoLines(item, &state->fonts.regular, maxWidth);
}

void OnAppResize(AppState *state)
{
    ForEachVisibleChild(state, &state->root, UpdateLines);
}

inline void ScrollBy(AppState *state, i32 delta)
{
    state->yOffset = ClampI32(state->yOffset + delta, 0, state->pageHeight - state->canvas.height);
}


void AppendPageHeight(AppState *state, Item *item, i32 level)
{
    if (item->newLinesCount == 1)
        state->pageHeight += state->fonts.regular.textMetric.tmHeight * LINE_HEIGHT;
    else
    {
        for (int i = 1; i <= item->newLinesCount; i++)
        {
            state->pageHeight += state->fonts.regular.textMetric.tmHeight;
        }
        state->pageHeight += state->fonts.regular.textMetric.tmHeight * (LINE_HEIGHT - 1);
    }
}

inline void UpdatePageHeight(AppState *state)
{
    state->pageHeight = 0;
    ForEachVisibleChild(state, &state->root, AppendPageHeight);
}


inline void HandleInput(AppState *state, MyInput *input)
{
    if(state->canvas.width != state->lastWidthRenderedWith)
    {
        OnAppResize(state);
        UpdatePageHeight(state);
        state->lastWidthRenderedWith = state->canvas.width;
    }

    if(input->wheelDelta)
        ScrollBy(state, -input->wheelDelta);

    if(input->keysPressed['L'])
        MoveCursor(state, CursorMove_Right);

    if(input->keysPressed['H'])
        MoveCursor(state, CursorMove_Left);
  
    if(input->keysPressed['J'])
        MoveCursor(state, CursorMove_Down);
    
    if(input->keysPressed['K'])
        MoveCursor(state, CursorMove_Up);

    if (input->keysPressed['D'])
        MoveSelectionBox(state, SelectionBox_Down);

    if (input->keysPressed['F'])
        MoveSelectionBox(state, SelectionBox_Up);

    if (input->keysPressed['S'])
    {
        if (MoveSelectionBox(state, SelectionBox_Left))
            UpdatePageHeight(state);
    }

    if (input->keysPressed['G'])
    {
        if (MoveSelectionBox(state, SelectionBox_Right))
            UpdatePageHeight(state);
    }
}

void RenderItem(AppState *state, Item *item, i32 level)
{
    FontData *font = &state->fonts.regular;
    i32 fontHeight = state->fonts.regular.textMetric.tmHeight;

    i32 lineHeightInPixels = fontHeight * LINE_HEIGHT;

    i32 isItemSelected = item == state->selectedItem;
    if (item == state->selectedItem)
    {
        i32 selectionColor = COLOR_SELECTION_BAR;
        i32 rectY = state->runningY - lineHeightInPixels / 2;
        i32 rectHeight = (item->newLinesCount + (LINE_HEIGHT - 1)) * fontHeight;

        // TODO: ugly change of offsets, will take place only in the next frame
        if (state->pageHeight > state->canvas.height)
        {
            if (rectY < 50)
                ScrollBy(state, rectY - 50);
            if (rectY + rectHeight > state->canvas.height - 50)
                ScrollBy(state, rectHeight);
        }

        DrawRect(&state->canvas, 0, rectY, state->canvas.width, rectHeight, selectionColor);
    }

    i32 textColor = isItemSelected ? COLOR_SELECTED_ITEM : COLOR_NORMAL_ITEM;
    i32 itemX = state->runningX + level * LEVEL_STEP;
    i32 itemY = state->runningY;
    DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE, 0x888888);

    if (item->childrenCount == 0)
        DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE - 4, COLOR_APP_BACKGROUND);

    i32 textX = itemX + ICON_SIZE / 2 + TEXT_TO_ICON;

    Start(FramePrintText);
    for (int i = 1; i <= item->newLinesCount; i++)
    {
        
        i32 textY = state->runningY - FONT_SIZE / 10 - 1;
        char *text = item->textBuffer.text + (item->newLines[i - 1] == 0 ? item->newLines[i - 1] : item->newLines[i - 1] + 1);
        i32 lineLength = item->newLines[i] - item->newLines[i - 1];
        DrawTextLeftCenter(&state->canvas, font, textX, textY, text, lineLength, textColor);

        if (state->isCursorVisible && item == state->selectedItem && state->cursorPos >= item->newLines[i - 1] && state->cursorPos < item->newLines[i])
        {
            i32 cursorPosOnLine = state->cursorPos - item->newLines[i - 1];
            DrawRect(&state->canvas, textX + GetTextWidth(font, text, cursorPosOnLine), textY - fontHeight / 2, 1, fontHeight, 0xffffff);
        }
        state->runningY += fontHeight;
    }
    state->runningY += fontHeight * (LINE_HEIGHT - 1);
    Stop(FramePrintText);
}

void UpdateAndDrawApp(AppState *state, MyInput *input)
{
    HandleInput(state, input);

    i32 lineHeightInPixels = state->fonts.regular.textMetric.tmHeight * LINE_HEIGHT;

    state->runningX = PAGE_PADDING;
    state->runningY = PAGE_PADDING + lineHeightInPixels / 2 - state->yOffset;
    
    ForEachVisibleChild(state, &state->root, RenderItem);

    if(state->pageHeight > state->canvas.height)
    {
        i32 scrollY = (i32)((float)state->yOffset * (float)(state->canvas.height / (float)state->pageHeight));
        i32 scrollHeight = state->canvas.height * state->canvas.height / state->pageHeight;
        i32 scrollWidth = 15;
        DrawRect(&state->canvas, state->canvas.width - scrollWidth, scrollY, scrollWidth, scrollHeight, 0x552D2E);
    }
}
