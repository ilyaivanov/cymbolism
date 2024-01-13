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

void InitEmptyBufferWithCapacity(StringBuffer *buffer, i32 capacity)
{
    buffer->capacity = capacity;
    buffer->text = AllocateMemory(buffer->capacity);
    buffer->length = 0;
}

inline void MoveBytesRight(char *ptr, int length) 
{
    for (int i = length - 1; i > 0; i--) {
        ptr[i] = ptr[i - 1];
    }
}

inline void MoveBytesLeft(char *ptr, int length) 
{
    for (int i = 0; i < length - 1; i++) {
        ptr[i] = ptr[i + 1];
    }
}

void InsertCharAt(StringBuffer *buffer, i32 at, i32 ch)
{
    if(buffer->length >= buffer->capacity)
    {
        char *currentStr = buffer->text;
        buffer->capacity *= 2;
        buffer->text = AllocateMemory(buffer->capacity);
        MoveMemory(buffer->text, currentStr, buffer->length);
        FreeMemory(currentStr);
    }

    buffer->length += 1;
    MoveBytesRight(buffer->text + at, buffer->length - at);
    *(buffer->text + at) = ch;
}

void RemoveCharAt(StringBuffer *buffer, i32 at)
{
    MoveBytesLeft(buffer->text + at, buffer->length - at);
    buffer->length--;
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

inline void ForEachChildLeveled(Item *parent, ForEachLeveledHandler *handler)
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

        for (int c = ChildCount(item) - 1; c >= 0; c--)
            stack[++currentItemInStack] = (ItemInStack){GetChildAt(item, c), current.level + 1};
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

void FreeItem(Item *item)
{
    FreeMemory(item->textBuffer.text);
    if(ChildCount(item) > 0)
        FreeMemory(item->childrenBuffer.children);
    FreeMemory(item);
}

Item* RemoveItem(Item *item)
{
    i32 index = GetItemIndex(item);
    Item* parent = item->parent;
    ForEachChild(item, FreeItem);
    RemoveChildFromParent(item);
    FreeItem(item);
    
    if(ChildCount(parent) == 0)
        return parent;
    if(index > 0)
        return GetChildAt(parent, index - 1);
    else 
        return GetChildAt(parent, index);
}


//
//
// Movement
//
//

void MoveItemDown(Item *item)
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

void MoveItemUp(Item *item)
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

void MoveItemRight(Item *item)
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

void MoveItemLeft(Item *item)
{
    Item *parent = item->parent;

    if(IsRoot(parent))
        return;
    
    i32 parentIndex = GetItemIndex(parent);
    RemoveChildFromParent(item);
    InsertChildAt(parent->parent, item, parentIndex + 1);
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
char *contentToSave = 0;
i32 currentContent = 0;
i32 contentSize = 0;

inline void AppendChar(char ch)
{
    *(contentToSave + currentContent) = ch;
    currentContent++;
}

void AppendItem(Item *item, i32 level)
{
    for(int i = 0; i < level*2; i++)
        AppendChar(' ');

    // GetBit check is technically redundant, but conveys more meaning
    // we check if any flags was set at runtime or item was saved with [ ] placeholder

    u32 isDone = GetBit(item->flags, ItemStateFlag_IsDone);
    u32 isOpen = GetBit(item->flags, ItemStateFlag_IsOpen);
    u32 makedWithPlaceholder = GetBit(item->flags, ItemStateFlag_IsSerializedWithPlaceholder);

    if(makedWithPlaceholder || (!isOpen && ChildCount(item) > 0) || isDone)
    {
        AppendChar('[');
        i32 hasAtLeastOneFlag = 0;
        if (isDone)
            AppendChar('y');

        if (!isOpen && ChildCount(item) > 0)
            AppendChar('h');

        if(!isDone && !(!isOpen && ChildCount(item) > 0))
            AppendChar(' ');


        AppendChar(']');
        AppendChar(' ');
    }

    for(int i = 0; i < item->textBuffer.length; i++)
        AppendChar(*(item->textBuffer.text + i));

    AppendChar('\n');
}

u32 SerializeState(Item *root, char *buffer, u32 bufferLength)
{
    contentToSave = buffer;
    contentSize = bufferLength;
    currentContent = 0;

    ForEachChildLeveled(root, AppendItem);

    return currentContent - 1;
}


#endif