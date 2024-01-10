#ifndef ITEM_C
#define ITEM_C

#include "math.c"
#include "memory.c"

// Depends only on the type, need to think, maybe extract win32.h? 
#include "win32.c"

#define IsRoot(item) (!item->parent)

typedef struct StringBuffer
{
    char *text;
    i32 capacity;
    i32 length;
} StringBuffer;

typedef struct ItemChildrenBuffer
{
    struct Item **children;
    i32 capacity;
    i32 length;
} ItemChildrenBuffer;


typedef enum ItemStateFlag
{
    ItemStateFlag_IsOpen,
    ItemStateFlag_IsDone,

    // indicated wheather I read this item as "[ ] My item" or "[y] My item" from a file
    // I don't want to obtruce the file by always placings [ ] for each item, only if any of the flags are set or it was like that in the file
    ItemStateFlag_IsSerializedWithPlaceholder,
} ItemStateFlag;




typedef struct Item
{
    struct Item *parent;
    ItemChildrenBuffer childrenBuffer;
    StringBuffer textBuffer;

    u32 flags; 
} Item;


//
// Flag bits
//
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
// Text buffer
//
void InitBuffer(StringBuffer *buffer, char* text, i32 sourceSize)
{
    buffer->capacity = sourceSize * 2;
    buffer->text = AllocateMemory(buffer->capacity);
    i32 targetIndex = 0;
    for (int sourceIndex = 0; sourceIndex < sourceSize; sourceIndex++)
    {
        char ch = *(text + sourceIndex);

        //TODO: need to figure this out on the parsing phase
        if(ch != '\r' && ch != '\n' && ch != '\0')
        {
            *(buffer->text + targetIndex) = ch;
            targetIndex++;
        }
    }
    buffer->length = targetIndex;
}

//
// Children
//


inline void InitChildren(Item* parent, i32 capacity)
{
    parent->childrenBuffer.length = 0;
    parent->childrenBuffer.capacity = capacity;
    parent->childrenBuffer.children = AllocateZeroedMemory(capacity, sizeof(Item*));
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

inline Item* GetChildAt(Item *parent, i32 index)
{
    return *(parent->childrenBuffer.children + index);
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


typedef void ForEachHandler(Item * item);
inline void ForEachChild(Item *parent, ForEachHandler *handler)
{
    Item *stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = ChildCount(parent) - 1; c >= 0; c--)
        stack[++currentItemInStack] = GetChildAt(parent, c);

    while (currentItemInStack >= 0)
    {
        Item *item = stack[currentItemInStack--];

        handler(item);

        for (int c = ChildCount(item) - 1; c >= 0; c--)
            stack[++currentItemInStack] = GetChildAt(item, c);
    }
}

typedef struct ItemInStack
{
    Item *ref;
    int level;
} ItemInStack;

typedef void ForEachLeveledHandler(Item * item, i32 level);
inline void ForEachVisibleChild(Item *parent, ForEachLeveledHandler *handler)
{
    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = ChildCount(parent) - 1; c >= 0; c--)
        stack[++currentItemInStack] = (ItemInStack){GetChildAt(parent, c), 0};

    while (currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        handler(current.ref, current.level);

        if (IsOpen(item))
        {
            for (int c = ChildCount(item) - 1; c >= 0; c--)
                stack[++currentItemInStack] = (ItemInStack){GetChildAt(item, c), current.level + 1};
        }
    }
}


//
//
// Traversing
//
//

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



//
//
// Deserialization
//
//


typedef struct ItemEntry
{
    i32 start, end, level;
    Item *item;
    u32 flags;
} ItemEntry;


inline void TrimEntry(ItemEntry *entry, char *content)
{
    while(((*(content + entry->start) == ' ') || (*(content + entry->start) == '\n')) && (entry->start != entry->end))
    {
        entry->start++;
    }

    while(((*(content + entry->end) == ' ')   || (*(content + entry->end) == '\n')) && (entry->start != entry->end))
    {
        entry->end--;
    }
    entry->end++;
}

void CloseIfEmpty(Item *item)
{
    if(ChildCount(item) == 0)
        SetIsOpen(item, 0);
}

void ParseFileContent(Item *root, FileContent file)
{
    //
    // Parse file
    //

    ItemEntry slices[512];
    i32 slicesCount = 0;

    i32 currentLineStart = 0;
    int i = 0;

    // State machines to the rescue? 
    i32 isLineEmpty = 1;
    i32 lineLevel = 0;
    i32 isReadingFlags = 0;
    i32 hasEndedReadingFlags = 0;
    i32 flagsEndAt = 0;
    u32 itemFlags = 0;

    u32 needToCloseItem = 0;

    for (; i < file.size; i++)
    {
        char ch = *(file.content + i);
        if (ch == '\r')
            continue;

        if (ch == '[' && !hasEndedReadingFlags)
        {
            isReadingFlags = 1;
            isLineEmpty = 0;
            SetBit(&itemFlags, ItemStateFlag_IsSerializedWithPlaceholder, 1);
        }
        else if (ch == ']' && isReadingFlags)
        {
            isReadingFlags = 0;
            hasEndedReadingFlags = 1;
            flagsEndAt = i + 1;

            if(!needToCloseItem)
                SetBit(&itemFlags, ItemStateFlag_IsOpen, 1);
        }
        else if (isReadingFlags)
        {
            if(ch == 'y')
                SetBit(&itemFlags, ItemStateFlag_IsDone, 1);

            if(ch == 'h')
                needToCloseItem = 1;
        }
        else if (ch == '\n' || i == file.size - 1)
        {
            if (!isLineEmpty)
            {
                if(flagsEndAt == 0)
                    SetBit(&itemFlags, ItemStateFlag_IsOpen, 1);

                u32 start = flagsEndAt == 0 ? currentLineStart : flagsEndAt;
                u32 end = i == file.size - 1 ? i + 1 : i;
                ItemEntry entry = {.start = start, .end = end, .level = lineLevel, .flags = itemFlags};
                TrimEntry(&entry, file.content);

                slices[slicesCount++] = entry;
            }

            currentLineStart = i + 1;
            isLineEmpty = 1;
            lineLevel = 0;
            isReadingFlags = 0;
            hasEndedReadingFlags = 0;
            flagsEndAt = 0;
            itemFlags = 0;
            needToCloseItem = 0;
        }
        else if (ch != ' ')
        {
            isLineEmpty = 0;
        }
        else if (isLineEmpty)
        {
            lineLevel++;
        }
    }

    //
    // Convert files slices into nested tree of items
    //

    ItemEntry stack[256] = {0};
    i32 stackLength = 0;

    slices[0].item = root;
    stack[stackLength++] = (ItemEntry){0, 0, -1, root};

    for (int i = 0; i < slicesCount; i++)
    {
        ItemEntry slice = slices[i];

        while (stack[stackLength - 1].level >= slice.level)
            stackLength--;

        ItemEntry parentSlice = stack[stackLength - 1];

        if (ChildCount(parentSlice.item) == 0)
            InitChildren(parentSlice.item, 4);

        slice.item = AllocateZeroedMemory(1, sizeof(Item));
        slice.item->flags = slice.flags;
        
        AppendChild(parentSlice.item, slice.item);
        stack[stackLength++] = slice;

        InitBuffer(&slice.item->textBuffer, file.content + slice.start, slice.end - slice.start);
    }

    ForEachChild(root, CloseIfEmpty);
}


//
//
// Serialization
//
//



#endif