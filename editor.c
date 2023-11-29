#include "drawing.c"
#include "types.h"
#include "item.c"
#include "selection.c"

#define BACKGROUND_COLOR_GREY 0x11

i32 iconR = 3;

Item root;
Item *selectedItem;

void InitApp()
{
    InitRoot(&root);

    selectedItem = root.children + 2;
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

void UpdateAndDrawApp(MyBitmap *bitmap)
{
    int x = padding;
    int y = padding + GetAscent();

    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    int currentItemIndex = -2;
    stack[++currentItemInStack] = (ItemInStack){&root, -1};

    while(currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];

        if(current.ref == selectedItem)
        {
            DrawRect(bitmap, 0, y - GetAscent(), bitmap->width, GetFontHeight() + 5, 0x454545);
        }

        if(current.ref->text) // Skip root items without a text
        {
            DrawLineAt(bitmap, x + STEP * current.level, y, current.ref);
            y += GetFontHeight() * 1.2;
        }

        if(current.ref->isOpen)
        {
            for (int c = current.ref->childrenCount - 1; c >= 0; c--)
            {
                stack[++currentItemInStack] = (ItemInStack){current.ref->children + c, current.level + 1};
            }
        }
    }
}


void HandleInput(MyInput* input)
{
    if(input->downPressed)
    {
        Item* itemBelow = GetItemBelow(selectedItem);
        if(itemBelow)
            selectedItem = itemBelow;
    }

    if(input->upPressed)
    {
        Item* itemAbove = GetItemAbove(selectedItem);
        if(itemAbove && itemAbove->parent)
            selectedItem = itemAbove;
    }
    
    if(input->leftPressed)
    {
        if(selectedItem->isOpen)
            selectedItem->isOpen = 0;
        else if(selectedItem->parent->parent)
            selectedItem = selectedItem->parent;
    }

    if(input->rightPressed)
    {
        if(!selectedItem->isOpen && selectedItem->childrenCount > 0)
            selectedItem->isOpen = 1;
        else if(selectedItem->childrenCount > 0) 
            selectedItem = selectedItem->children;
    }
}
