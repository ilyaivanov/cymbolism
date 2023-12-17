#include <stdio.h>
#include "types.h"
#include "text.c"
#include "item.c"
#include "selection.c"

#define BACKGROUND_COLOR_GREY 0x18
#define COLOR_APP_BACKGROUND 0x181818
#define COLOR_SELECTION_BAR 0x2F2F2F
#define COLOR_SELECTED_ITEM 0xffffff
#define COLOR_NORMAL_ITEM   0xdddddd


#define FONT_SIZE 16
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
    InitFontSystem(&state->fonts.regular, FONT_SIZE, FONT_FAMILY);
    InitRoot(&state->root);

    state->selectedItem = state->root.children;
}

void OnAppResize(AppState *state)
{
    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    stack[++currentItemInStack] = (ItemInStack){&state->root, -1};

    while (currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        if (item->parent)
        {
            //TODO: remove duplication from the UI view
            i32 maxWidth = state->canvas.width - PAGE_PADDING * 2 - (ICON_SIZE / 2 + TEXT_TO_ICON) - current.level * LEVEL_STEP;
            SplitTextIntoLines(item, &state->fonts.regular, maxWidth);
        }

        if (item->isOpen)
        {
            for (int c = item->childrenCount - 1; c >= 0; c--)
            {
                stack[++currentItemInStack] = (ItemInStack){item->children + c, current.level + 1};
            }
        }
    }
}

inline void ScrollBy(AppState *state, i32 delta)
{
    i32 nextOffset = state->yOffset + delta;

    if (nextOffset < 0)
        nextOffset = 0;

    if (nextOffset > state->pageHeight - state->canvas.height)
        nextOffset = state->pageHeight - state->canvas.height;

    state->yOffset = nextOffset;
}

i32 lastWidth = 0;

inline void HandleInput(AppState *state, MyInput *input)
{
    if(state->canvas.width != lastWidth)
    {
        OnAppResize(state);
        lastWidth = state->canvas.width;
    }

    if(input->wheelDelta)
        ScrollBy(state, -input->wheelDelta);

    if (input->keysPressed['J'])
    {
        Item *itemBelow = GetItemBelow(state->selectedItem);
        if (itemBelow)
            state->selectedItem = itemBelow;
    }

    if (input->keysPressed['K'])
    {
        Item *itemAbove = GetItemAbove(state->selectedItem);
        if (itemAbove && itemAbove->parent)
            state->selectedItem = itemAbove;
    }

    if (input->keysPressed['H'])
    {
        if (state->selectedItem->isOpen)
            state->selectedItem->isOpen = 0;
        else if (state->selectedItem->parent->parent)
            state->selectedItem = state->selectedItem->parent;
    }

    if (input->keysPressed['L'])
    {
        if (!state->selectedItem->isOpen && state->selectedItem->childrenCount > 0)
            state->selectedItem->isOpen = 1;
        else if (state->selectedItem->childrenCount > 0)
            state->selectedItem = state->selectedItem->children;
    }
}

void UpdateAndDrawApp(AppState *state, MyInput *input)
{
    HandleInput(state, input);

    i32 lineHeightInPixels = state->fonts.regular.textMetric.tmHeight * LINE_HEIGHT;

    i32 x = PAGE_PADDING;
    i32 y = PAGE_PADDING + lineHeightInPixels / 2 - state->yOffset;

    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    stack[++currentItemInStack] = (ItemInStack){&state->root, -1};

    while(currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;


        if(item->parent) // Skip root items without a parent
        {
            i32 isItemSelected = item == state->selectedItem;
            if(item == state->selectedItem)
            {
                i32 selectionColor = COLOR_SELECTION_BAR;
                i32 rectY = y - lineHeightInPixels / 2;
                i32 rectHeight = (item->newLinesCount + (LINE_HEIGHT - 1)) * state->fonts.regular.textMetric.tmHeight;
                DrawRect(&state->canvas, 0, rectY, state->canvas.width, rectHeight, selectionColor);
            }

            i32 textColor = isItemSelected ? COLOR_SELECTED_ITEM : COLOR_NORMAL_ITEM;
            i32 itemX = x + current.level * LEVEL_STEP;
            i32 itemY = y;
            DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE, 0x888888);

            if(item->childrenCount == 0)
                DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE - 4, COLOR_APP_BACKGROUND);

            i32 textX = itemX + ICON_SIZE / 2 + TEXT_TO_ICON;

            if (item->newLinesCount == 1)
            {
                i32 textY = y - FONT_SIZE / 10 - 1;
                DrawTextLeftCenter(&state->canvas, &state->fonts.regular, textX, textY, item->textBuffer.text, item->textBuffer.length, textColor);
                y += lineHeightInPixels;
            }
            else
            {

                for (int i = 1; i <= item->newLinesCount; i++)
                {
                    i32 textY = y - FONT_SIZE / 10 - 1;
                    char *text = item->textBuffer.text + (item->newLines[i - 1] == 0 ? item->newLines[i - 1] : item->newLines[i - 1] + 1);
                    i32 lineLength = item->newLines[i] - item->newLines[i - 1];
                    DrawTextLeftCenter(&state->canvas, &state->fonts.regular, textX, textY, text, lineLength, textColor);
                    y += state->fonts.regular.textMetric.tmHeight;
                }
                y += state->fonts.regular.textMetric.tmHeight * (LINE_HEIGHT - 1);
            }

        }


        if(item->isOpen)
        {
            for (int c = item->childrenCount - 1; c >= 0; c--)
            {
                stack[++currentItemInStack] = (ItemInStack){item->children + c, current.level + 1};
            }
        }
    }

    state->pageHeight = y + state->yOffset;
    if(state->pageHeight > state->canvas.height)
    {
        i32 scrollY = (i32)((float)state->yOffset * (float)(state->canvas.height / (float)state->pageHeight));
        i32 scrollHeight = state->canvas.height * state->canvas.height / state->pageHeight;
        i32 scrollWidth = 15;
        DrawRect(&state->canvas, state->canvas.width - scrollWidth, scrollY, scrollWidth, scrollHeight, 0x552D2E);
    }
}
