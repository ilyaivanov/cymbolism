#include "drawing.c"
#include "types.h"
#include "item.c"

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

        for (int c = 0; c < item->childrenCount; c++)
        {
            items[++currentItem] = item->children + c;
        }
    }

    return res;
}

void DrawLineAt(MyBitmap *bitmap, int x, int baselineY, Item *item)
{
    int bottom = baselineY + GetDescent();
    int cy = bottom - GetFontHeight() / 2 - iconR + 2;

    DrawRect(bitmap, x - iconR - 12, cy, iconR * 2, iconR * 2, 0xcccccccc);


    if (item->childrenCount > 0)
    {
        i32 numberOfVisibleChildren = GetVisibleChildrenCount(item);
        i32 linePadding = 10;
        DrawRect(bitmap, x - iconR / 2 - 12, cy + iconR * 2 + linePadding, 2, numberOfVisibleChildren * GetFontHeight() * 1.2 - iconR * 2 - linePadding * 2, 0xcc555555);
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


        for (int c = current.ref->childrenCount - 1; c >= 0; c--)
        {
            stack[++currentItemInStack] = (ItemInStack){current.ref->children + c, current.level + 1};
        }
    }
}
