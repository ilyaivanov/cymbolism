#include "types.h"
#include "item.c"
#include "selection.c"

#define BACKGROUND_COLOR_GREY 0x11

#define ICON_SIZE 6 
#define SMALL_ICON_SIZE 4 

#define padding 50
#define STEP 40
#define TEXT_TO_ICON_DISTANCE 10
#define SMALL_ICON_TO_ICON_DISTANCE 8

#define LINE_TO_ICON_DISTANCE 10
#define FONT_SIZE 13
#define FONT_FAMILY "Segoe UI"

//Colors
#define SELECTION_BAR_EDIT_MODE 0x99800080
#define SELECTION_BAR_NORMAL_MODE 0x99454545

void InitApp(AppState *state)
{
    InitFontSystem(&state->fonts.regular, FONT_SIZE, FONT_FAMILY);
    InitRoot(&state->root);

    state->selectedItem = state->root.children;
}


i32 GetVisibleChildrenCount(Item* parent)
{
    i32 res = 0;
    Item *items [128] = {0};
    int currentItem = -1;
    
    items[++currentItem] = parent;

    while(currentItem >= 0)
    {
        Item *item = items[currentItem--];
        res++;

        if(item->isOpen)
        {
            for (int c = 0; c < item->childrenCount; c++)
            {
                items[++currentItem] = item->children + c;
            }
        }
    }

    return res;
}

void DrawItem(AppState *state, int x, int y, Item *item)
{
    DrawSquareAtCenter(&state->canvas, x, y, ICON_SIZE, 0xcccccccc);

    DrawTextLeftCentered(&state->canvas, &state->fonts.regular, x + ICON_SIZE / 2 + TEXT_TO_ICON_DISTANCE, y - 2, item->textBuffer.text, 0xffffffff);

    if (item->isOpen)
    {
        i32 numberOfVisibleChildren = GetVisibleChildrenCount(item);
        DrawRect(&state->canvas,
                 x - 1,
                 y + ICON_SIZE / 2 + LINE_TO_ICON_DISTANCE,
                 2,
                 numberOfVisibleChildren * GetFontHeight(&state->fonts.regular) * 1.2 - ICON_SIZE * 2 - LINE_TO_ICON_DISTANCE * 2, 0xcc454545);
    }
    else if (item->childrenCount > 0)
    {
        DrawSquareAtCenter(&state->canvas, x - ICON_SIZE / 2 - SMALL_ICON_TO_ICON_DISTANCE - SMALL_ICON_SIZE / 2, y, SMALL_ICON_SIZE, 0xcccccccc);
    }
}



void UpdateAndDrawApp(AppState *state)
{
    int x = padding;
    int y = padding;

    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    stack[++currentItemInStack] = (ItemInStack){&state->root, -1};

    while(currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        int height = state->fonts.regular.textMetric.tmHeight;
        if(item == state->selectedItem)
        {
            i32 selectionColor = state->editMode == EditMode_Edit ? SELECTION_BAR_EDIT_MODE : SELECTION_BAR_NORMAL_MODE;
            DrawRect(&state->canvas, 0, y - height / 2, state->canvas.width, height, selectionColor);

            if(state->editMode == EditMode_Edit)
            {
                i32 selectionWidth = GetTextWidth(&state->fonts.regular, item->textBuffer.text, state->cursorPosition) - 1;
                i32 cursorX = x + STEP * current.level + selectionWidth + ICON_SIZE / 2 + TEXT_TO_ICON_DISTANCE;
                i32 cursorY = y - height / 2;
                DrawRect(&state->canvas, cursorX ,cursorY, 2, height, 0xffffffff);
            }
        }

        if(item->parent) // Skip root items without a parent
        {
            DrawItem(state, x + STEP * current.level, y, item);
            y += height * 1.2;
        }

        if(item->isOpen)
        {
            for (int c = item->childrenCount - 1; c >= 0; c--)
            {
                stack[++currentItemInStack] = (ItemInStack){item->children + c, current.level + 1};
            }
        }
    }

    char * modeLabel = state->editMode == EditMode_Edit ? "Edit" : "Normal";
    DrawTextLeftBottom(&state->canvas, &state->fonts.regular, 10, state->canvas.height - 10, modeLabel, 0xffaaaaaa);
}


void HandleInput(MyInput* input, AppState *state)
{

    if (state->editMode == EditMode_Normal)
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
        if (input->keysPressed['I'])
            {state->editMode = EditMode_Edit;
            state->cursorPosition = 0;}
    } else
    {
        for (int i = 'A'; i <= 'Z'; i++)
        {
            if (input->keysPressed[i])
            {
                i8 ch = input->isPressed[VK_SHIFT] ? i : i - ('A' - 'a');
                InsertCharAt(state->selectedItem, state->cursorPosition, ch);
                state->cursorPosition += 1;
            }
        }
        if(input->keysPressed[VK_SPACE]){
            InsertCharAt(state->selectedItem, state->cursorPosition, ' ');
            state->cursorPosition += 1;
        }
    }

    if(input->keysPressed[VK_ESCAPE] || input->keysPressed[VK_RETURN])
        state->editMode = EditMode_Normal;
}
