#include "types.h"

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

void CloseIfEmpty(AppState * state, Item * item)
{
    if(ChildCount(item) == 0)
        SetIsOpen(item, 0);
}

void ParseFileContent(AppState *state, Item *root, FileContent file)
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

    ForEachActualChild(state, root, CloseIfEmpty);
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

void AppendItem(AppState*state, Item*item, i32 level)
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

u32 SerializeState(AppState *state, char *buffer, u32 bufferLength)
{
    contentToSave = buffer;
    contentSize = bufferLength;
    currentContent = 0;

    ForEachActualChildLeveled(state, &state->root, AppendItem);

    return currentContent - 1;
}