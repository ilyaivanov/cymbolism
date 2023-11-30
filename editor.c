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

// char *fontName = "Courier New";
char *fontName = "Segoe UI";
// char *fontName = "Consolas";
// char *fontName = "Arial";

//monospaced
// char *fontName = "Lucida Console";

void InitApp(AppState *state)
{
    InitFontSystem(&state->fonts.regular, FONT_SIZE, fontName);
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

    DrawTextLeftCentered(&state->canvas, &state->fonts.regular, x + ICON_SIZE / 2 + TEXT_TO_ICON_DISTANCE, y - 2, item->text, 0xffffffff);

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
            DrawRect(&state->canvas, 0, y - height / 2, state->canvas.width, height, 0x454545);
        }

        if(item->text) // Skip root items without a text
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
}


void HandleInput(MyInput* input, AppState *state)
{
    if(input->downPressed)
    {
        Item* itemBelow = GetItemBelow(state->selectedItem);
        if(itemBelow)
            state->selectedItem = itemBelow;
    }

    if(input->upPressed)
    {
        Item* itemAbove = GetItemAbove(state->selectedItem);
        if(itemAbove && itemAbove->parent)
            state->selectedItem = itemAbove;
    }
    
    if(input->leftPressed)
    {
        if(state->selectedItem->isOpen)
            state->selectedItem->isOpen = 0;
        else if(state->selectedItem->parent->parent)
            state->selectedItem = state->selectedItem->parent;
    }

    if(input->rightPressed)
    {
        if(!state->selectedItem->isOpen && state->selectedItem->childrenCount > 0)
            state->selectedItem->isOpen = 1;
        else if(state->selectedItem->childrenCount > 0) 
            state->selectedItem = state->selectedItem->children;
    }
}
