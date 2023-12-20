#include "types.h"

//
// Growable children list
//
void InitChildren(Item* parent, i32 capacity)
{
    parent->childrenBuffer.length = 0;
    parent->childrenBuffer.capacity = capacity;
    parent->childrenBuffer.children = AllocateZeroedMemory(capacity, sizeof(Item*));
}

void AppendChild(Item* parent, Item* child)
{
    ItemChildrenBuffer *buffer = &parent->childrenBuffer;
    if(buffer->length == buffer->capacity)
    {
        buffer->capacity *= 2;
        Item **newChildren = AllocateZeroedMemory(buffer->capacity, sizeof(Item*));
        MoveMemory(newChildren, buffer->children, buffer->length * sizeof(Item*));
        FreeMemory(buffer->children);
        buffer->children = newChildren;
    }
    buffer->children[buffer->length++] = child;
    child->parent = parent;
}

i32 RemoveChildFromParent(Item* child)
{
    i32 index = GetItemIndex(child);
    if(index < ChildCount(child->parent) - 1)
    {
        Item **dest = child->parent->childrenBuffer.children + index;
        Item **src = child->parent->childrenBuffer.children + index + 1;
        size_t len = sizeof(Item*) * (child->parent->childrenBuffer.length - index);
        MoveMemory(dest, src, len);
    }    

    child->parent->childrenBuffer.length--;
    return index;
}

inline Item* GetChildAt(Item *parent, i32 index)
{
    return *(parent->childrenBuffer.children + index);
}
inline void SetChildAt(Item *parent, i32 index, Item *child)
{
     *(parent->childrenBuffer.children + index) = child;
}
inline int ChildCount(Item *parent)
{
     return parent->childrenBuffer.length;
}

inline i32 GetItemIndex(Item *item)
{
    Item *parent = item->parent;
    for (int i = 0; i < ChildCount(parent); i++)
    {
        if (GetChildAt(parent, i) == item)
            return i;
    }
    return -1;
}





//
//
//

typedef void ForEachLeveledHandler(AppState *state, Item * item, i32 level);
//TODO: check macros if function call would be too slow
//This will be moved inside UI model
inline void ForEachVisibleChild(AppState *state, Item *parent, ForEachLeveledHandler *handler)
{
    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = ChildCount(parent) - 1; c >= 0; c--)
        stack[++currentItemInStack] = (ItemInStack){GetChildAt(parent, c), 0};

    while (currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        handler(state, current.ref, current.level);

        if (item->isOpen)
        {
            for (int c = ChildCount(item) - 1; c >= 0; c--)
                stack[++currentItemInStack] = (ItemInStack){GetChildAt(item, c), current.level + 1};
        }
    }
}


typedef void ForEachHandler(AppState *state, Item * item);
inline void ForEachActualChild(AppState *state, Item *parent, ForEachHandler *handler)
{
    Item *stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = ChildCount(parent) - 1; c >= 0; c--)
        stack[++currentItemInStack] = GetChildAt(parent, c);

    while (currentItemInStack >= 0)
    {
        Item *item = stack[currentItemInStack--];

        handler(state, item);

        for (int c = ChildCount(item) - 1; c >= 0; c--)
            stack[++currentItemInStack] = GetChildAt(item, c);
    }
}

inline void ForEachActualChildLeveled(AppState *state, Item *parent, ForEachLeveledHandler *handler)
{
    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = ChildCount(parent) - 1; c >= 0; c--)
        stack[++currentItemInStack] = (ItemInStack){GetChildAt(parent, c), 0};

    while (currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        handler(state, current.ref, current.level);

        for (int c = ChildCount(item) - 1; c >= 0; c--)
            stack[++currentItemInStack] = (ItemInStack){GetChildAt(item, c), current.level + 1};
    }
}





Item *GetItemBelow(Item *item)
{
    if (item->isOpen)
    {
        return GetChildAt(item, 0);
    }
    else
    {
        Item *parent = item->parent;
        i32 itemIndex = GetItemIndex(item);
        if (itemIndex < ChildCount(parent) - 1)
        {
            return GetChildAt(parent, itemIndex + 1);
        }
        else
        {
            while (parent->parent && GetItemIndex(parent) == ChildCount(parent->parent) - 1 && parent->isOpen)
                parent = parent->parent;
            if (parent->parent)
                return GetChildAt(parent->parent, GetItemIndex(parent) + 1);
        }
    }
    return 0;
}

Item* GetItemAbove(Item * item)
{
    Item *parent = item->parent;
    i32 itemIndex = GetItemIndex(item);
    if(itemIndex == 0)
        return parent;
    Item *prevItem = GetChildAt(parent, itemIndex - 1);
    //looking for the most nested item
    while(prevItem->isOpen)
        prevItem = GetChildAt(prevItem, ChildCount(prevItem) - 1);
    return prevItem;

    return 0;
}


void FreeItem(AppState *state, Item *item)
{
    FreeMemory(item->textBuffer.text);
    if(ChildCount(item) > 0)
        FreeMemory(item->childrenBuffer.children);
    FreeMemory(item);
}

Item* RemoveItem(AppState *state, Item *item)
{
    i32 index = GetItemIndex(item);
    Item* parent = item->parent;
    ForEachActualChild(state, item, FreeItem);
    RemoveChildFromParent(item);
    FreeItem(state, item);
    
    if(ChildCount(parent) == 0)
        return parent;
    if(index > 0)
        return GetChildAt(parent, index - 1);
    else 
        return GetChildAt(parent, index);
}

void MoveItemDown(AppState *state, Item *item)
{
    Item *parent = item->parent;
    i32 index = GetItemIndex(item);
    if(index < ChildCount(parent) - 1)
    {
        Item *temp = GetChildAt(parent, index + 1);
        SetChildAt(parent, index + 1, item);
        SetChildAt(parent, index, temp);
    }
}

void MoveItemUp(AppState *state, Item *item)
{
    Item *parent = item->parent;
    i32 index = GetItemIndex(item);
    if(index > 0)
    {
        Item *temp = GetChildAt(parent, index - 1);
        SetChildAt(parent, index - 1, item);
        SetChildAt(parent, index, temp);
    }
}

typedef struct ItemEntry { i32 start, end, level; Item *item; } ItemEntry;

void InitRootFromFile(Item *root, FileContent file)
{
    //
    // Parse file
    //

    ItemEntry slices[512];
    i32 slicesCount = 0;

    i32 currentLineStart = 0;
    int i = 0;
    i32 isLineEmpty = 1;
    i32 lineLevel = 0;

    for(; i < file.size; i++)
    {
        char ch = *(file.content + i);
        if(ch == '\r')
            continue;
        
        if(ch == '\n')
        {
            if(!isLineEmpty)
                slices[slicesCount++] = (ItemEntry){currentLineStart + lineLevel, i, lineLevel, 0};

            currentLineStart = i + 1;
            isLineEmpty = 1;
            lineLevel = 0;
        }
        else if(ch != ' ')
        {
            isLineEmpty = 0;
        }
        else if (isLineEmpty)
        {
            lineLevel++;
        }
    }
    slices[slicesCount++] = (ItemEntry){currentLineStart + lineLevel, i + 1, lineLevel, 0};

    //
    // Convert files slices into nested tree of items
    //

    ItemEntry stack[256] = {0};
    i32 stackLength = 0;

    slices[0].item = root;
    stack[stackLength++] = (ItemEntry){0, 0, -1, root};

    

    for(int i = 0; i < slicesCount; i++)
    {
        ItemEntry slice = slices[i];

        while(stack[stackLength - 1].level >= slice.level)
            stackLength--;

        ItemEntry parentSlice = stack[stackLength - 1];
        
        if(ChildCount(parentSlice.item) == 0)
        {
            parentSlice.item->isOpen = 1;
            InitChildren(parentSlice.item, 4);
        }

        slice.item = AllocateZeroedMemory(1, sizeof(Item));
        AppendChild(parentSlice.item, slice.item);
        stack[stackLength++] = slice;

        InitBuffer(&slice.item->textBuffer, file.content + slice.start, slice.end - slice.start);
    }
}