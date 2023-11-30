#include "drawing.c"
#include "types.h"
#include "item.c"
#include "selection.c"

#define BACKGROUND_COLOR_GREY 0x11

#define iconR 3

void InitApp(AppState *state)
{
    InitRoot(&state->root);

    state->selectedItem = state->root.children + 2;
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

void DrawLineAt(MyBitmap *bitmap, int x, int baselineY, Item *item)
{
    int bottom = baselineY + GetDescent();
    int middleY = bottom - GetFontHeight() / 2;
    int cy = middleY - iconR + 2;

    DrawRect(bitmap, x - iconR - 12, cy, iconR * 2, iconR * 2, 0xcccccccc);


    if (item->isOpen)
    {
        i32 numberOfVisibleChildren = GetVisibleChildrenCount(item);
        i32 linePadding = 20;
        DrawRect(bitmap, x - iconR / 2 - 12, cy + iconR * 2 + linePadding, 2, numberOfVisibleChildren * GetFontHeight() * 1.2 - iconR * 2 - linePadding * 2, 0xcc555555);
    } else if (item->childrenCount > 0)
    {
        DrawRect(bitmap, x - 2 - 24, middleY, 4, 4, 0xcccccccc);

    }

    char * text = item->text;
    int len = strlen(text);
    for (int i = 0; i < len; i += 1)
    {
        char codepoint = *text;
        MyBitmap *glyphBitmap = GetGlyphBitmap(codepoint);
        DrawTextureTopLeft(bitmap, glyphBitmap, x, baselineY - glyphBitmap->height + GetDescent(), 0xffffffff);

        char nextCodepoint = *(text + 1);
        x += glyphBitmap->width + GetKerningValue(codepoint, nextCodepoint);
        text++;
    }
}

int padding = 50;
int STEP = 40;

void UpdateAndDrawApp(MyBitmap *bitmap, AppState *state)
{
    int x = padding;
    int y = padding + GetAscent();

    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    int currentItemIndex = -2;
    stack[++currentItemInStack] = (ItemInStack){&state->root, -1};

    while(currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        if(item == state->selectedItem)
        {
            DrawRect(bitmap, 0, y - GetAscent(), bitmap->width, GetFontHeight() + 5, 0x454545);
        }

        if(item->text) // Skip root items without a text
        {
            DrawLineAt(bitmap, x + STEP * current.level, y, item);
            y += GetFontHeight() * 1.2;
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
