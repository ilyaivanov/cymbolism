#include <stdio.h>
#include "types.h"
#include "text.c"
#include "item.c"
#include "selection.c"

#define BACKGROUND_COLOR_GREY 0x11
#define COLOR_APP_BACKGROUND 0x111111
#define COLOR_SELECTION_BAR 0x272930
#define COLOR_SELECTED_ITEM 0xffffff
#define COLOR_NORMAL_ITEM   0xdddddd


#define FONT_SIZE 16
#define FONT_FAMILY "Segoe UI"
#define ICON_SIZE 10
#define TEXT_TO_ICON 15
#define LEVEL_STEP 40
#define LINE_HEIGHT 1.2f
#define PAGE_PADDING 30


void InitApp(AppState *state)
{
    InitFontSystem(&state->fonts.regular, FONT_SIZE, FONT_FAMILY);
    InitRoot(&state->root);

    state->selectedItem = state->root.children;
}


void UpdateAndDrawApp(AppState *state, MyInput *input)
{
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

    i32 lineHeightInPixels = state->fonts.regular.textMetric.tmHeight * LINE_HEIGHT;

    i32 x = PAGE_PADDING;
    i32 y = PAGE_PADDING + lineHeightInPixels / 2;

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
                // i32 selectionColor = state->editMode == EditMode_Edit ? SELECTION_BAR_EDIT_MODE : COLOR_SELECTION_BAR;
                i32 selectionColor = COLOR_SELECTION_BAR;
                DrawRect(&state->canvas, 0, y - lineHeightInPixels / 2, state->canvas.width, lineHeightInPixels, selectionColor);

                // if(state->editMode == EditMode_Edit)
                // {
                //     i32 selectionWidth = GetTextWidth(&state->fonts.regular, item->textBuffer.text, state->cursorPosition) - 1;
                //     i32 cursorX = x + STEP * current.level + selectionWidth + ICON_SIZE / 2 + TEXT_TO_ICON_DISTANCE;
                //     i32 cursorY = y - height / 2;
                //     DrawRect(&state->canvas, cursorX ,cursorY, 2, height, 0xffffffff);
                // }
            }

            i32 textColor = isItemSelected ? COLOR_SELECTED_ITEM : COLOR_NORMAL_ITEM;
            i32 itemX = x + current.level * LEVEL_STEP;
            i32 itemY = y;
            DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE, 0x888888);

            if(item->childrenCount == 0)
                DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE - 4, COLOR_APP_BACKGROUND);

            i32 textX = itemX + ICON_SIZE / 2 + TEXT_TO_ICON;
            i32 textY = itemY - FONT_SIZE / 10 - 1;
            DrawTextLeftCenter(&state->canvas, &state->fonts.regular, textX, textY, item->textBuffer.text, item->textBuffer.length, textColor);

            y += lineHeightInPixels;
        }


        if(item->isOpen)
        {
            for (int c = item->childrenCount - 1; c >= 0; c--)
            {
                stack[++currentItemInStack] = (ItemInStack){item->children + c, current.level + 1};
            }
        }
    }
}
