#include "types.h"

i32 GetItemIndex(Item *item)
{
    Item *parent = item->parent;
    for (int i = 0; i < parent->childrenCount; i++)
    {
        if (parent->children + i == item)
            return i;
    }
    return -1;
}


Item *GetItemBelow(Item *item)
{
    if (item->childrenCount > 0)
    {
        return item->children;
    }
    else
    {
        Item *parent = item->parent;
        i32 itemIndex = GetItemIndex(item);
        if (itemIndex < parent->childrenCount - 1)
        {
            return parent->children + itemIndex + 1;
        }
        else
        {
            while (parent->parent && GetItemIndex(parent) == parent->parent->childrenCount - 1)
                parent = parent->parent;
            if (parent->parent)
                return parent->parent->children + GetItemIndex(parent) + 1;
        }
    }
    return 0;
}

Item* GetItemAbove(Item * item){
    Item *parent = item->parent;
    i32 itemIndex = GetItemIndex(item);
    if(itemIndex == 0)
        return parent;
    Item *prevItem = parent->children + itemIndex - 1;
    //looking for the most nested item
    while(prevItem->childrenCount > 0)
        prevItem = prevItem->children + prevItem->childrenCount - 1;
    return prevItem;

    return 0;
}