#include "types.h"

#define IsRoot(item) (!item->parent)

// GetBit and SetBit is lousy copied from with small modifications
// https://stackoverflow.com/a/47990/1283124
inline GetBit(u32 flags, u32 bitPosition)
{
    return (flags >> bitPosition) & (u32)1;
}

inline SetBit(u32 *flags, u32 bitPosition, u32 val)
{
    *flags = (*flags & ~((u32)1 << bitPosition)) | ((u32)val << bitPosition);
}

inline i32 IsOpen(Item *item)
{
    return GetBit(item->flags, ItemStateFlag_IsOpen);
}

inline void SetIsOpen(Item *item, i32 isOpen)
{
    SetBit(&item->flags, ItemStateFlag_IsOpen, isOpen);
}

inline i32 IsDone(Item *item)
{
    return GetBit(item->flags, ItemStateFlag_IsDone);
}

inline void SetIsDone(Item *item, i32 isDone)
{
    SetBit(&item->flags, ItemStateFlag_IsDone, isDone);
}


//
// Growable children list
//


inline void InitChildren(Item* parent, i32 capacity)
{
    parent->childrenBuffer.length = 0;
    parent->childrenBuffer.capacity = capacity;
    parent->childrenBuffer.children = AllocateZeroedMemory(capacity, sizeof(Item*));
}

inline void InitChildrenIfEmptyWithDefaultCapacity(Item *item)
{
    if (ChildCount(item) == 0)
        InitChildren(item, 4);
}

void ExpandBufferIfFull(Item* item)
{
    ItemChildrenBuffer *buffer = &item->childrenBuffer;
    if(buffer->length == buffer->capacity)
    {
        buffer->capacity *= 2;
        Item **newChildren = AllocateZeroedMemory(buffer->capacity, sizeof(Item*));
        MoveMemory(newChildren, buffer->children, buffer->length * sizeof(Item*));
        FreeMemory(buffer->children);
        buffer->children = newChildren;
    }
}

void AppendChild(Item *parent, Item* child)
{
    ExpandBufferIfFull(parent);
    parent->childrenBuffer.children[parent->childrenBuffer.length++] = child;
    child->parent = parent;
}

void InsertChildAt(Item* parent, Item* child, i32 index)
{
    ExpandBufferIfFull(parent);

    ItemChildrenBuffer *buffer = &parent->childrenBuffer;
    Item **from = buffer->children + index;
    Item **to = buffer->children + index + 1;
    MoveMemory(to, from, (buffer->length - index) * sizeof(Item*));

    buffer->children[index] = child;
    child->parent = parent;
    buffer->length++;
}

inline i32 RemoveChildFromParent(Item* child)
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
    if(ChildCount(child->parent) == 0)
        SetIsOpen(child->parent, 0);

    child->parent = 0;
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

        if (IsOpen(item))
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
    if (IsOpen(item))
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
            while (!IsRoot(parent) && GetItemIndex(parent) == ChildCount(parent->parent) - 1 && IsOpen(parent))
                parent = parent->parent;
            if (!IsRoot(parent))
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
    while(IsOpen(prevItem))
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

void MoveItemRight(AppState *state, Item *item)
{
    i32 index = GetItemIndex(item);
    if(index > 0)
    {
        Item *prevItem = GetChildAt(item->parent, index - 1);
        
        RemoveChildFromParent(item);
        InitChildrenIfEmptyWithDefaultCapacity(prevItem);
        AppendChild(prevItem, item);

        SetIsOpen(prevItem, 1);
    }
}

void MoveItemLeft(AppState *state, Item *item)
{
    Item *parent = item->parent;

    if(IsRoot(parent))
        return;
    
    i32 parentIndex = GetItemIndex(parent);
    RemoveChildFromParent(item);
    InsertChildAt(parent->parent, item, parentIndex + 1);
}

