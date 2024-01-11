#include <gl/gl.h>
#include <stdio.h>
#include <math.h>
#include "math.c"
#include "layout.c"
#include "font.c"
#include "input.c"
#include "item.c"
#include "selection.c"
#include "cursor.c"

V3f BG_COLOR = HexColor(0x181818);
#define paddingV PX(20)
#define paddingH PX(16)
#define iconSize PX(7)
#define itemStep PX(20)
#define iconBorderWidth PX(3)
#define iconToText PX(6)
#define scrollWidth PX(8)

typedef struct App
{
    Layout header;
    Layout body;
    Layout footer;
} App;

App app;
Item *selectedItem;
CursorState cursor;

void Init(Item *root)
{
    selectedItem = GetChildAt(root, 0);

    InitFonts();
}

inline void BuildLayout(V2i screenSize)
{
    f32 headerHeight = PX(0);
    f32 footerHeight = PX(0);
    // f32 leftSidebarWidth = ((sinf(appTime / 500) + 1) * 50);

    f32 bodyWidth = screenSize.x;
    f32 bodyHeight = screenSize.y - (headerHeight + footerHeight);

    app.footer.width = screenSize.x;
    app.footer.height = footerHeight;

    app.body.x = 0;
    app.body.y = app.footer.height;
    app.body.width = bodyWidth; // - ((sinf(appTime / 2500) + 1) * 250);
    app.body.height = bodyHeight;

    app.header.width = screenSize.x;
    app.header.height = headerHeight;
    app.header.y = app.body.y + app.body.height;
}

void SaveState(Item *root)
{
    u32 contentSize = 40 * 1024; //assumes 40kb is enought, just for now 

    char *buffer = VirtualAllocateMemory(contentSize);
    u32 bytesWritten = SerializeState(root, buffer, contentSize);
    WriteMyFile(FILE_PATH, buffer, bytesWritten);

    VirtualFreeMemory(buffer);
}


void DrawItem(Item *item, i32 level)
{
    glColor3f(88.0f / 255.0f, 88.0f / 255.0f, 88.0f / 255.0f);

    float itemX = paddingH + level * itemStep;
    float itemY = app.body.y + app.body.height - app.body.offsetY - app.body.runningY - paddingV;
    DrawSquareCentered(itemX, itemY, iconSize);

    if (item->childrenBuffer.length == 0)
    {
        glColor3f(BG_COLOR.r, BG_COLOR.g, BG_COLOR.g);
        DrawSquareCentered(itemX, itemY, iconSize - iconBorderWidth);
    }

    float textX = itemX + iconSize + iconToText;
    // picked by eye for better font allignment
    float textManualShift = currentFont->textMetric.tmHeight / 15;
    float textY = itemY + textManualShift;
    float width = app.body.width - textX - paddingH - scrollWidth;

    glColor3f(1, 1, 1);

    u32 newLines[32] = {0};
    u32 newLinesCount = 0;

    float paragraphMargin = currentFont->textMetric.tmHeight * 0.2;
    float paragraphHeight = DrawParagraph(item->textBuffer.text, item->textBuffer.length, textX, textY, width, newLines, &newLinesCount) + paragraphMargin;

    if (item == selectedItem)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        if(cursor.isVisible)
            DrawCursor(item, &cursor, textX, textY, newLines, &newLinesCount);

        if(cursor.isEditing)
            glColor4f(0.4, 1, 1, 0.3f);
        else 
            glColor4f(1, 1, 1, 0.2f);

        DrawRectGLBottomLeft(app.body.x, itemY - paragraphHeight + currentFont->textMetric.tmHeight / 2 + paragraphMargin / 2, app.body.width, paragraphHeight);
        glDisable(GL_BLEND);
    }

    app.body.runningY += paragraphHeight;
}
void ChangeSelection(Item *item)
{
    if (item && !IsRoot(item))
        selectedItem = item;
}

void HandleInput(Item *root, UserInput *input)
{
    if (IsMouseOverCurrentLayout(&app.body, input) && app.body.pageHeight > 0 && app.body.pageHeight > app.body.height)
    {
        // pageHeight is calculated on the previous frame, this might be limiting
        app.body.offsetY = Clampf32(app.body.offsetY + input->zDeltaThisFrame, -(app.body.pageHeight - app.body.height), 0);
    }

    if (cursor.isEditing)
    {
        if (input->keysPressedhisFrame[VK_ESCAPE] || input->keysPressedhisFrame[VK_RETURN])
        {
            cursor.isEditing = 0;
        }


        InsertChar(&cursor, input, selectedItem);

         if (input->keysPressedhisFrame[VK_BACK] && cursor.cursorPos > 0)
        {
            RemoveCharAt(&selectedItem->textBuffer, cursor.cursorPos - 1);
            cursor.cursorPos--;
        }

        // copy-pasted from the else block, extract a function
        else if (input->keysPressedhisFrame['J'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Down);
        else if (input->keysPressedhisFrame['K'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Up);
        else if (input->keysPressedhisFrame['H'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Left);
        else if (input->keysPressedhisFrame['L'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Right);
        else if (input->keysPressedhisFrame['W'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_JumpOneWordForward);
        else if (input->keysPressedhisFrame['B'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_JumpOneWordBackward);
    }
    else
    {
        if (input->keysPressedhisFrame['J'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Down);
        else if (input->keysPressedhisFrame['K'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Up);
        else if (input->keysPressedhisFrame['H'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Left);
        else if (input->keysPressedhisFrame['L'] && input->keyboardState[VK_CONTROL])
            MoveCursor(&cursor, selectedItem, CursorMove_Right);
        else if (input->keysPressedhisFrame['J'] && input->keyboardState[VK_MENU])
            MoveItemDown(selectedItem);
        else if (input->keysPressedhisFrame['K'] && input->keyboardState[VK_MENU])
            MoveItemUp(selectedItem);
        else if (input->keysPressedhisFrame['H'] && input->keyboardState[VK_MENU])
            MoveItemLeft(selectedItem);
        else if (input->keysPressedhisFrame['L'] && input->keyboardState[VK_MENU])
            MoveItemRight(selectedItem);

        else if (input->keysPressedhisFrame['J'])
            MoveSelectionBox(&selectedItem, &cursor, SelectionBox_Down);
        else if (input->keysPressedhisFrame['K'])
            MoveSelectionBox(&selectedItem, &cursor, SelectionBox_Up);
        else if (input->keysPressedhisFrame['H'])
            MoveSelectionBox(&selectedItem, &cursor, SelectionBox_Left);
        else if (input->keysPressedhisFrame['L'])
            MoveSelectionBox(&selectedItem, &cursor, SelectionBox_Right);

        else if (input->keysPressedhisFrame['D'])
            selectedItem = RemoveItem(selectedItem);
        else if (input->keysPressedhisFrame['O'])
        {
            Item *item = AllocateZeroedMemory(1, sizeof(Item));
            InitEmptyBufferWithCapacity(&item->textBuffer, 10);
            i32 currentIndex = GetItemIndex(selectedItem);

            if (input->keyboardState[VK_CONTROL])
            {
                InitChildrenIfEmptyWithDefaultCapacity(selectedItem);
                InsertChildAt(selectedItem, item, 0);
                SetIsOpen(selectedItem, 1);
            }
            else
            {
                i32 targetIndex = input->keyboardState[VK_SHIFT] ? currentIndex + 1 : currentIndex;
                InsertChildAt(selectedItem->parent, item, targetIndex);
            }
            selectedItem = item;
            cursor.isEditing = 1;
            cursor.isVisible = 1;
            cursor.cursorPos = 0;
        } else if (input->keysPressedhisFrame['I']){
            cursor.isEditing = 1;
            cursor.isVisible = 1;
        } else if (input->keysPressedhisFrame['W'])
            MoveCursor(&cursor, selectedItem, CursorMove_JumpOneWordForward);
        else if (input->keysPressedhisFrame['B'])
            MoveCursor(&cursor, selectedItem, CursorMove_JumpOneWordBackward);
        else if (input->keysPressedhisFrame['S'] && input->keyboardState[VK_CONTROL])
            SaveState(root);

    }
}

void DrawUI(V2i screenSize, Item *root, UserInput *input)
{
    HandleInput(root, input);

    BuildLayout(screenSize);

    glClearColor(BG_COLOR.r, BG_COLOR.g, BG_COLOR.g, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    FillLayout(&app.header, (V3f)HexColor(0x232323));
    FillLayout(&app.footer, (V3f)HexColor(0x444444));

    ClipLayout(&app.body);

    glColor4f(1, 1, 1, 1);

    currentFont = &regularFont;
    app.body.runningY = 0;

    ForEachVisibleChild(root, DrawItem);

    app.body.pageHeight = app.body.runningY + paddingV;
    DrawScrollbar(&app.body, scrollWidth);
    EndCliping();
}