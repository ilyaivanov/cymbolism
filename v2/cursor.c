#ifndef CURSOR_C
#define CURSOR_C

#include "math.c"
#include "item.c"

// Cursor handles movement inside the Item unable to go outside of the item bounds for now
// Selection box handles a higher level movement throught the item tree structure

typedef enum CursorMovement
{
    CursorMove_Right,
    CursorMove_Left,
    CursorMove_Down,
    CursorMove_Up,
    CursorMove_JumpOneWordForward,
    CursorMove_JumpOneWordBackward,
} CursorMovement;

typedef struct CursorState
{
    i32 cursorPos;
    i32 isVisible;
    i32 isEditing;
} CursorState;

i32 GetNextWordIndex(Item *item, i32 currentPos)
{
    for (int i = currentPos + 1; i < item->textBuffer.length - 1; i++)
    {
        if (*(item->textBuffer.text + i) == ' ')
            return i + 1;
    }
    return item->textBuffer.length;
}
i32 GetPrevWordIndex(Item *item, i32 currentPos)
{
    for (int i = currentPos - 2; i > 0; i--)
    {
        if (*(item->textBuffer.text + i) == ' ')
            return i + 1;
    }
    return 0;
}

inline void GetLineNumberAndOffset(CursorState *cursor, Item* selectedItem, const u32 *newLines, const u32 *newLinesCount, u32 *lineNumber, u32 *offset)
{
    for(int i = 0; i < *newLinesCount; i++)
    {
        if(newLines[i] >= cursor->cursorPos){
            *lineNumber = i;
            *offset = i == 0 ? 0 : (newLines[i - 1] + 1);
            break;
        }
    }
}

inline i32 ClampCursor(i32 val, Item *item) 
{
    return Clampi32(val, 0, item->textBuffer.length);
}


// void MoveCursorDown(AppState *state)
// {
//     Item *item = state->selectedItem;
//     for (int i = 1; i <= item->newLinesCount; i++)
//     {
//         if (state->cursorPos >= item->newLines[i - 1] && state->cursorPos < item->newLines[i])
//         {
//             if (i != item->newLinesCount)
//             {
//                 i32 lineOffset = state->cursorPos - item->newLines[i - 1];
//                 state->cursorPos = item->newLines[i] + lineOffset + (i == 1 ? 1 : 0);
//                 break;
//             }
//         }
//     }
// }

// void MoveCursorUp(AppState *state)
// {
//     FontData *font = &state->fonts.regular;
//     Item *item = state->selectedItem;
//     for (int i = 1; i <= item->newLinesCount; i++)
//     {
//         if (state->cursorPos >= item->newLines[i - 1] && state->cursorPos < item->newLines[i])
//         {
//             if (i > 1)
//             {
//                 i32 lineOffset = state->cursorPos - item->newLines[i - 1];
//                 // TODO: will need to stabilize vertical cursor movement.
//                 // Also introduce "desired cursor locaiton" which won't change during vertical movement, but change during horizontal
//                 // i32 offsetWidth = GetTextWidth(font, item->textBuffer.text + item->newLines[i - 1], lineOffset);
//                 state->cursorPos = item->newLines[i - 2] + lineOffset - (i == 2 ? -1 : 0);
//                 break;
//             }
//         }
//     }
// }




void MoveCursor(CursorState *cursor, Item* selectedItem, CursorMovement movement)
{
    cursor->isVisible = 1;

    if (movement == CursorMove_Right)
        cursor->cursorPos = ClampCursor(cursor->cursorPos + 1, selectedItem);

    else if (movement == CursorMove_Left)
        cursor->cursorPos = ClampCursor(cursor->cursorPos - 1, selectedItem);

    // else if (movement == CursorMove_Down)
    //     MoveCursorDown(state);
    // else if (movement == CursorMove_Up)
    //     MoveCursorUp(state);
    else if (movement == CursorMove_JumpOneWordForward)
        cursor->cursorPos = GetNextWordIndex(selectedItem, cursor->cursorPos);
    else if (movement == CursorMove_JumpOneWordBackward)
        cursor->cursorPos = GetPrevWordIndex(selectedItem, cursor->cursorPos);
}

void InsertChar(CursorState *cursor, UserInput *input, Item *selectedItem)
{
    for (int i = 0; i < input->charEventsThisFrameCount; i++)
    {
        InsertCharAt(&selectedItem->textBuffer, cursor->cursorPos, input->charEventsThisFrame[i]);
        cursor->cursorPos++;
    }
}

void DrawCursor(Item *item, CursorState *cursor, float textX, float textY, const u32 *newLines, const u32 *newLinesCount)
{
    glColor3f(1, 1, 1);

    i32 cursorLine = 0;
    i32 lineStartsAt = 0;
    GetLineNumberAndOffset(cursor, item, newLines, newLinesCount, &cursorLine, &lineStartsAt);

    float cursorWidth = 2;
    float cursorX = textX + GetTextWidth(item->textBuffer.text + lineStartsAt, cursor->cursorPos - lineStartsAt) - cursorWidth / 2;
    float cursorY = textY - currentFont->textMetric.tmHeight / 2 - cursorLine * currentFont->textMetric.tmHeight;
    DrawRectGLBottomLeft(cursorX, cursorY, cursorWidth, currentFont->textMetric.tmHeight);
}
#endif