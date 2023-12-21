#include <stdio.h>
#include "types.h"
#include "text.c"
#include "number.c"
#include "item.c"
#include "cursorAndSelection.c"

#define BACKGROUND_COLOR_GREY 0x18
#define COLOR_APP_BACKGROUND 0x181818
#define COLOR_MENU_BACKGROUND 0x262626
#define COLOR_SELECTION_BAR_NORMAL_MODE 0x2F2F2F
#define COLOR_SELECTION_BAR_INSERT_MODE 0x2F5F5F

// Typography colors
#define COLOR_SELECTED_ITEM 0xffffff
#define COLOR_NORMAL_ITEM   0xdddddd
#define COLOR_DONE_ITEM     0x999999


#define FONT_SIZE 14
#define FONT_FAMILY "Segoe UI"
#define ICON_SIZE 10
#define TEXT_TO_ICON 15
#define LEVEL_STEP 40
#define LINE_HEIGHT 1.2f
#define PAGE_PADDING 30
#define BOTTOM_PADDING 50


void SplitTextIntoLines(Item *item, FontData *font, u32 maxWidth)
{
    int currentLine = 0;
    int runningWidth = 0;
    int wordFirstLetterIndex = 0;
    int wordLength = 0;

    for (int i = 0; i < item->textBuffer.length; i += 1)
    {
        i8 ch = *(item->textBuffer.text + i);
        
        if ((ch == ' ' || i == item->textBuffer.length - 1) && runningWidth >= maxWidth)
        {
            item->newLines[++currentLine] = wordFirstLetterIndex - 1;        
            runningWidth = wordLength;
            wordFirstLetterIndex = i + 1;
            wordLength = 0;
        }
        else 
        {
            int w = GetGlyphWidth(font, ch);
            if (ch == ' ')
            {
                wordFirstLetterIndex = i + 1;
                wordLength = 0;
            }
            else
            {
                wordLength += w;
            }
            runningWidth += w;
        }
    }

    item->newLines[++currentLine] = item->textBuffer.length;   
    item->newLinesCount = currentLine;
}

void InitApp(AppState *state)
{
    Start(StartUp);
    
    InitFontSystem(&state->fonts.regular, FONT_SIZE, FONT_FAMILY);
    
    // InitRoot(&state->root);
    FileContent file = ReadMyFileImp("..\\data1.txt");
    InitRootFromFile(&state->root, file);
    VirtualFreeMemory(file.content);


    state->selectedItem = GetChildAt(&state->root, 0);
    state->isFileSaved = 1;

    Stop(StartUp);

    PrintStartupResults();
}

char *contentToSave = 0;
i32 currentContent = 0;
i32 contentSize = 0;

void AppendItem(AppState*state, Item*item, i32 level)
{
    for(int i = 0; i < level*2; i++){
        *(contentToSave + currentContent) = ' ';
        currentContent++;
    }

    for(int i = 0; i < item->textBuffer.length; i++)
    {
        *(contentToSave + currentContent) = *(item->textBuffer.text + i);
        currentContent++;
    }

    *(contentToSave + currentContent) = '\n';
    currentContent++;
}

void SaveState(AppState *state)
{
    contentSize = 20 * 1024;
    currentContent = 0;
    contentToSave = AllocateMemory(contentSize);

    ForEachActualChildLeveled(state, &state->root, AppendItem);

    WriteMyFile("..\\data1.txt", contentToSave, currentContent - 1);

    FreeMemory(contentToSave);
    state->isFileSaved = 1;
}

inline void MarkFileUnsaved(AppState *state)
{
    state->isFileSaved = 0;
}

void UpdateLines(AppState *state, Item *item, i32 level)
{
    i32 maxWidth = state->canvas.width - PAGE_PADDING * 2 - (ICON_SIZE / 2 + TEXT_TO_ICON) - level * LEVEL_STEP;
    SplitTextIntoLines(item, &state->fonts.regular, maxWidth);
}

void OnAppResize(AppState *state)
{
    ForEachVisibleChild(state, &state->root, UpdateLines);
}

inline void ScrollTo(AppState *state, i32 val)
{
    state->yOffset = ClampI32(val, 0, state->pageHeight - state->canvas.height);
}

inline void ScrollBy(AppState *state, i32 delta)
{
    ScrollTo(state, state->yOffset + delta);
}

inline void CheckScrollOffset(AppState * state, i32 rectRunningY, i32 rectHeight)
{
    i32 rectAbsolutePos = rectRunningY + state->yOffset;
    i32 rectAbsoluteBottom = rectAbsolutePos + rectHeight;
    if(rectAbsoluteBottom > state->yOffset + state->canvas.height - 60)
        ScrollTo(state, rectAbsoluteBottom - state->canvas.height + 60);
    else if (rectAbsolutePos < state->yOffset + 60)
        ScrollTo(state, rectAbsolutePos - 60);
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
        OnAppResize(state);
        UpdatePageHeight(state);
        state->lastWidthRenderedWith = state->canvas.width;
    }
    if (input->wheelDelta)
        ScrollBy(state, -input->wheelDelta);

    if (state->editMode == EditorMode_Normal)
    {
        if (input->keysPressed['S'] && input->isPressed[VK_CONTROL])
            SaveState(state);

        else if (input->keysPressed['L'] && input->isPressed[VK_CONTROL])
            MoveCursor(state, CursorMove_Right);

        else if (input->keysPressed['H'] && input->isPressed[VK_CONTROL])
            MoveCursor(state, CursorMove_Left);

        else if (input->keysPressed['J'] && input->isPressed[VK_CONTROL])
            MoveCursor(state, CursorMove_Down);

        else if (input->keysPressed['K'] && input->isPressed[VK_CONTROL])
            MoveCursor(state, CursorMove_Up);

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

        else if (input->keysPressed['J'])
            MoveSelectionBox(state, SelectionBox_Down);

        else if (input->keysPressed['K'])
            MoveSelectionBox(state, SelectionBox_Up);

        else if (input->keysPressed['H'])
        {
            if (MoveSelectionBox(state, SelectionBox_Left))
                UpdatePageHeight(state);
        }

        else if (input->keysPressed['L'])
        {
            if (MoveSelectionBox(state, SelectionBox_Right))
                UpdatePageHeight(state);
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
            state->cursorPos = 0;
            state->editMode = EditorMode_Insert;
            state->isCursorVisible = 1;
            OnAppResize(state);
            UpdatePageHeight(state);
            MarkFileUnsaved(state);
        } 
        else if (input->keysPressed[VK_RETURN] && input->isPressed[VK_CONTROL])
        {
            SetIsDone(state->selectedItem, !IsDone(state->selectedItem));
            MarkFileUnsaved(state);
        }
    }
    else if (state->editMode == EditorMode_Insert)
    {
        for (int i = 0; i < input->charEventsThisFrameCount; i++)
        {
            InsertCharAt(&state->selectedItem->textBuffer, state->cursorPos, input->charEventsThisFrame[i]);
            state->cursorPos++;
            MarkFileUnsaved(state);
        }

        if(input->keysPressed[VK_BACK] && state->cursorPos > 0)
        {
            RemoveCharAt(&state->selectedItem->textBuffer, state->cursorPos - 1);
            state->cursorPos--;
            MarkFileUnsaved(state);
        }
        // I don't need to update all items, but now I don't know x level of a selected item. 
        // This will be solved once I introduce statefull UI model
        ForEachVisibleChild(state, &state->root, UpdateLines);

        if(input->keysPressed[VK_ESCAPE] || input->keysPressed[VK_RETURN])
            state->editMode = EditorMode_Normal;
    }
}
void RenderItem(AppState *state, Item *item, i32 level)
{
    FontData *font = &state->fonts.regular;
    i32 fontHeight = state->fonts.regular.textMetric.tmHeight;

    i32 lineHeightInPixels = fontHeight * LINE_HEIGHT;

    i32 isItemSelected = item == state->selectedItem;
    if (item == state->selectedItem)
    {
        i32 selectionColor = state->editMode == EditorMode_Insert ? COLOR_SELECTION_BAR_INSERT_MODE : COLOR_SELECTION_BAR_NORMAL_MODE;
        i32 rectY = state->runningY - lineHeightInPixels / 2;
        i32 rectHeight = (item->newLinesCount + (LINE_HEIGHT - 1)) * fontHeight;

        if (state->pageHeight > state->canvas.height)
            CheckScrollOffset(state, rectY, rectHeight);

        DrawRect(&state->canvas, 0, rectY, state->canvas.width, rectHeight, selectionColor);
    }

    
    i32 itemX = state->runningX + level * LEVEL_STEP;
    i32 itemY = state->runningY;
    DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE, 0x888888);

    if (ChildCount(item) == 0)
        DrawSquareAtCenter(&state->canvas, itemX, itemY, ICON_SIZE - 4, COLOR_APP_BACKGROUND);
    
    i32 isDone = IsDone(item);
    i32 textColor = isItemSelected ? COLOR_SELECTED_ITEM : isDone ? COLOR_DONE_ITEM : COLOR_NORMAL_ITEM;

    if (isDone)
    {
        MyBitmap *t = &state->fonts.regular.checkmark;
        DrawTextureCentered(&state->canvas, t, state->canvas.width - PAGE_PADDING, itemY, textColor);
    }


    i32 textX = itemX + ICON_SIZE / 2 + TEXT_TO_ICON;

    Start(FramePrintText);
    for (int i = 1; i <= item->newLinesCount; i++)
    {
        i32 textY = state->runningY - FONT_SIZE / 10 - 1;
        char *text = item->textBuffer.text + (item->newLines[i - 1] == 0 ? item->newLines[i - 1] : item->newLines[i - 1] + 1);
        i32 lineLength = item->newLines[i] - item->newLines[i - 1];
        
        Start(FramePrintTextDrawTexture);
        DrawTextLeftCenter(&state->canvas, font, textX, textY, text, lineLength, textColor);
        Stop(FramePrintTextDrawTexture);

        if ((state->isCursorVisible || state->editMode == EditorMode_Insert) && item == state->selectedItem && state->cursorPos >= item->newLines[i - 1] && state->cursorPos <= item->newLines[i])
        {
            i32 cursorPosOnLine = state->cursorPos - item->newLines[i - 1];
            DrawRect(&state->canvas, textX + GetTextWidth(font, text, cursorPosOnLine), textY - fontHeight / 2, 1, fontHeight, 0xffffff);
        }
        state->runningY += fontHeight;
    }
    state->runningY += fontHeight * (LINE_HEIGHT - 1);
    Stop(FramePrintText);
}

void UpdateAndDrawApp(AppState *state, MyInput *input)
{
    HandleInput(state, input);

    i32 lineHeightInPixels = state->fonts.regular.textMetric.tmHeight * LINE_HEIGHT;

    state->runningX = PAGE_PADDING;
    state->runningY = PAGE_PADDING + lineHeightInPixels / 2 - state->yOffset;
    
    ForEachVisibleChild(state, &state->root, RenderItem);

    if(state->pageHeight > state->canvas.height)
    {
        i32 scrollY = (i32)((float)state->yOffset * (float)(state->canvas.height / (float)state->pageHeight));
        i32 scrollHeight = state->canvas.height * state->canvas.height / state->pageHeight;
        i32 scrollWidth = 15;
        DrawRect(&state->canvas, state->canvas.width - scrollWidth, scrollY, scrollWidth, scrollHeight, 0x552D2E);
    }



    // Drawing status bar at the bottom
    FontData *font = &state->fonts.regular;
    i32 labelsC = 0x888888;
    i32 padding = 10;

    i32 r = font->textMetric.tmHeight + padding * 1.3;
    DrawRect(&state->canvas, 0, state->canvas.height - r, state->canvas.width, r, COLOR_MENU_BACKGROUND);
    
    char *label = state->editMode == EditorMode_Normal ? "Normal" : "Insert";
    DrawTextLeftBottom(&state->canvas, font, padding, state->canvas.height - padding, label, strlen(label), labelsC);

    char *label2 = state->isFileSaved ? "Saved" : "Modified";
    DrawTextCenterBottom(&state->canvas, font, state->canvas.width / 2, state->canvas.height - padding, label2, strlen(label2), labelsC);
    
    
    char buff[256];
    sprintf(buff, "Cur %d | %d length | %d capacity",
            state->cursorPos, state->selectedItem->textBuffer.length, state->selectedItem->textBuffer.capacity);


    DrawTextRightBottom(&state->canvas, font, state->canvas.width - padding, state->canvas.height - padding, &buff[0], strlen(&buff[0]), labelsC);


}


