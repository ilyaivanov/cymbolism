#include "types.h"

// Cursor handles movement inside the Item unable to go outside of the item bounds for now
// Selection box handles a higher level movement throught the item tree structure

#define CLAMP_CURSOR(val) (ClampI32((val), 0, state->selectedItem->textBuffer.length))

typedef enum CursorMovement
{
    CursorMove_Right,
    CursorMove_Left,
    CursorMove_Down,
    CursorMove_Up,
    CursorMove_JumpOneWordForward,
    CursorMove_JumpOneWordBackward,
} CursorMovement;

typedef enum SelectionBoxMovement
{
    SelectionBox_Right,
    SelectionBox_Left,
    SelectionBox_Down,
    SelectionBox_Up,
} SelectionBoxMovement;

void MoveCursorDown(AppState *state)
{
    Item *item = state->selectedItem;
    for (int i = 1; i <= item->newLinesCount; i++)
    {
        if (state->cursorPos >= item->newLines[i - 1] && state->cursorPos < item->newLines[i])
        {
            if (i != item->newLinesCount)
            {
                i32 lineOffset = state->cursorPos - item->newLines[i - 1];
                state->cursorPos = item->newLines[i] + lineOffset + (i == 1 ? 1 : 0);
                break;
            }
        }
    }
}

void MoveCursorUp(AppState *state)
{
    FontData *font = &state->fonts.regular;
    Item *item = state->selectedItem;
    for (int i = 1; i <= item->newLinesCount; i++)
    {
        if (state->cursorPos >= item->newLines[i - 1] && state->cursorPos < item->newLines[i])
        {
            if (i > 1)
            {
                i32 lineOffset = state->cursorPos - item->newLines[i - 1];
                // TODO: will need to stabilize vertical cursor movement.
                // Also introduce "desired cursor locaiton" which won't change during vertical movement, but change during horizontal
                // i32 offsetWidth = GetTextWidth(font, item->textBuffer.text + item->newLines[i - 1], lineOffset);
                state->cursorPos = item->newLines[i - 2] + lineOffset - (i == 2 ? -1 : 0);
                break;
            }
        }
    }
}

void MoveCursor(AppState *state, CursorMovement movement)
{
    state->isCursorVisible = 1;

    if (movement == CursorMove_Right)
    {
        state->cursorPos = CLAMP_CURSOR(state->cursorPos + 1);
    }
    else if (movement == CursorMove_Left)
    {
        state->cursorPos = CLAMP_CURSOR(state->cursorPos - 1);
    }
    else if (movement == CursorMove_Down)
        MoveCursorDown(state);
    else if (movement == CursorMove_Up)
        MoveCursorUp(state);
    else if (movement == CursorMove_JumpOneWordForward)
        state->cursorPos = GetNextWordIndex(state->selectedItem, state->cursorPos);
    else if (movement == CursorMove_JumpOneWordBackward)
        state->cursorPos = GetPrevWordIndex(state->selectedItem, state->cursorPos);
}


//TODO: draw cursor