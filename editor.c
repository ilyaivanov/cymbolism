#include <stdio.h>
#include "types.h"
#include "text.c"
#include "number.c"
#include "textReflow.c"
#include "item.c"
#include "serialization.c"
#include "cursorAndSelection.c"
#include "ui.c"


#define FILE_PATH "..\\data.txt"

void InitApp(AppState *state)
{
    Start(StartUp);
    
    InitFontSystem(&state->fonts.regular, FONT_SIZE, FONT_FAMILY);
    
    FileContent file = ReadMyFileImp(FILE_PATH);
    ParseFileContent(state, &state->root, file);
    VirtualFreeMemory(file.content);

    state->focusedItem = &state->root;
    state->selectedItem = GetChildAt(&state->root, 0);
    state->isFileSaved = 1;

    Stop(StartUp);

    PrintStartupResults();
}

void SaveState(AppState *state)
{
    u32 contentSize = 40 * 1024; //assumes 40kb is enought, just for now 

    char *buffer = VirtualAllocateMemory(contentSize);
    u32 bytesWritten = SerializeState(state, buffer, contentSize);
    WriteMyFile(FILE_PATH, buffer, bytesWritten);

    VirtualFreeMemory(buffer);
    state->isFileSaved = 1;
}

inline void MarkFileUnsaved(AppState *state)
{
    state->isFileSaved = 0;
}


void AppendPageHeight(AppState *state, Item *item, i32 level)
{
    if (item->newLinesCount == 1)
        state->pageHeight += state->fonts.regular.textMetric.tmHeight * LINE_HEIGHT;
    else
    {
        for (int i = 1; i <= item->newLinesCount; i++)
        {
            state->pageHeight += state->fonts.regular.textMetric.tmHeight;
        }
        state->pageHeight += state->fonts.regular.textMetric.tmHeight * (LINE_HEIGHT - 1);
    }
}

inline void UpdatePageHeight(AppState *state)
{
    state->pageHeight = 0;
    ForEachVisibleChild(state, &state->root, AppendPageHeight);
    state->pageHeight += state->fonts.regular.textMetric.tmHeight * (LINE_HEIGHT);


    if(state->pageHeight <= state->canvas.height)
        state->yOffset = 0;

    state->pageHeight += BOTTOM_PADDING;
}


inline void HandleInput(AppState *state, MyInput *input)
{
    if (state->canvas.width != state->lastWidthRenderedWith)
    {
        UpdatePageHeight(state);
        state->lastWidthRenderedWith = state->canvas.width;
    }
    if (input->wheelDelta)
        ScrollBy(state, -input->wheelDelta);

    if (input->keysPressed['L'] && input->isPressed[VK_CONTROL])
        MoveCursor(state, CursorMove_Right);

    else if (input->keysPressed['H'] && input->isPressed[VK_CONTROL])
        MoveCursor(state, CursorMove_Left);

    else if (input->keysPressed['J'] && input->isPressed[VK_CONTROL])
        MoveCursor(state, CursorMove_Down);

    else if (input->keysPressed['K'] && input->isPressed[VK_CONTROL])
        MoveCursor(state, CursorMove_Up);

    if (state->editMode == EditorMode_Normal)
    {
        if(input->keysPressed['F'] && input->isPressed[VK_SHIFT])
        {
            if(!IsRoot(state->focusedItem))
                state->focusedItem = state->focusedItem->parent;

            //TOOD: extra logic here to check if selected item is visible
        }
        else if(input->keysPressed['F'] )
        {
            state->focusedItem = state->selectedItem;
        }
        else if (input->keysPressed['S'] && input->isPressed[VK_CONTROL])
            SaveState(state);

        else if (input->keysPressed['J'] && input->isPressed[VK_MENU])
        {
            MoveItemDown(state, state->selectedItem);
            MarkFileUnsaved(state);
        }

        else if (input->keysPressed['K'] && input->isPressed[VK_MENU])
        {
            MoveItemUp(state, state->selectedItem);
            MarkFileUnsaved(state);
        }

        else if (input->keysPressed['H'] && input->isPressed[VK_MENU])
        {
            MoveItemLeft(state, state->selectedItem);
            MarkFileUnsaved(state);
        }

        else if (input->keysPressed['L'] && input->isPressed[VK_MENU])
        {
            MoveItemRight(state, state->selectedItem);
            MarkFileUnsaved(state);
        }

        // need to think how to improve this and remove redundant negation check. 
        // this is done so that when cursor is moved I won't move selection
        else if (input->keysPressed['J'] && !input->isPressed[VK_CONTROL])
            MoveSelectionBox(state, SelectionBox_Down);

        else if (input->keysPressed['K'] && !input->isPressed[VK_CONTROL])
            MoveSelectionBox(state, SelectionBox_Up);

        else if (input->keysPressed['H'] && !input->isPressed[VK_CONTROL])
        {
            if (MoveSelectionBox(state, SelectionBox_Left))
                UpdatePageHeight(state);
        }

        else if (input->keysPressed['L'] && !input->isPressed[VK_CONTROL])
        {
            if (MoveSelectionBox(state, SelectionBox_Right))
            {
                UpdatePageHeight(state);
            }
        }
        else if (input->keysPressed['I'])
        {
            state->editMode = EditorMode_Insert;
            state->isCursorVisible = 1;
        }
        else if (input->keysPressed['D'])
        {
            state->selectedItem = RemoveItem(state, state->selectedItem);
            UpdatePageHeight(state);
            MarkFileUnsaved(state);
        }
        else if (input->keysPressed['O'])
        {
            Item *item = AllocateZeroedMemory(1, sizeof(Item));
            InitEmptyBufferWithCapacity(&item->textBuffer, 10);
            i32 currentIndex = GetItemIndex(state->selectedItem);

            if(input->isPressed[VK_CONTROL])
            {
                InitChildrenIfEmptyWithDefaultCapacity(state->selectedItem);
                InsertChildAt(state->selectedItem, item, 0);
                SetIsOpen(state->selectedItem, 1);
            }
            else 
            {
                i32 targetIndex = input->isPressed[VK_SHIFT] ? currentIndex + 1 : currentIndex;
                InsertChildAt(state->selectedItem->parent, item, targetIndex);
            }
            state->selectedItem = item;
            state->editMode = EditorMode_Insert;
            state->cursorPos = 0;
            state->isCursorVisible = 1;
            UpdatePageHeight(state);
            MarkFileUnsaved(state);
        } 
        else if (input->keysPressed['W'])
           MoveCursor(state, CursorMove_JumpOneWordForward);
        else if (input->keysPressed['B'])
           MoveCursor(state, CursorMove_JumpOneWordBackward);
        else if (input->keysPressed[VK_RETURN] && input->isPressed[VK_CONTROL])
        {
            SetIsDone(state->selectedItem, !IsDone(state->selectedItem));
            MarkFileUnsaved(state);
        }
    }
    else if (state->editMode == EditorMode_Insert)
    {
        if (input->keysPressed['W'] && input->isPressed[VK_CONTROL])
            MoveCursor(state, CursorMove_JumpOneWordForward);
        else if (input->keysPressed['B'] && input->isPressed[VK_CONTROL])
            MoveCursor(state, CursorMove_JumpOneWordBackward);
        else if (input->keysPressed[VK_ESCAPE] || input->keysPressed[VK_RETURN])
        {
            state->editMode = EditorMode_Normal;
        }
        else
        {

            for (int i = 0; i < input->charEventsThisFrameCount; i++)
            {
                InsertCharAt(&state->selectedItem->textBuffer, state->cursorPos, input->charEventsThisFrame[i]);
                state->cursorPos++;
                MarkFileUnsaved(state);
            }

            if (input->keysPressed[VK_BACK] && state->cursorPos > 0)
            {
                RemoveCharAt(&state->selectedItem->textBuffer, state->cursorPos - 1);
                state->cursorPos--;
                MarkFileUnsaved(state);
            }
        }
    }
}
void UpdateAndDrawApp(AppState *state, MyInput* input)
{
    HandleInput(state, input);

    DrawApp(state, input);
}