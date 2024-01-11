#include "math.c"
#include "item.c"


typedef enum SelectionBoxMovement
{
    SelectionBox_Right,
    SelectionBox_Left,
    SelectionBox_Down,
    SelectionBox_Up,
} SelectionBoxMovement;


void MoveSelectionBox(Item **selectedItem, SelectionBoxMovement movement)
{
    // isCursorVisible = 0;
    // cursorPos = 0;

    if (movement == SelectionBox_Right)
    {
        if (!IsOpen(*selectedItem) && ChildCount(*selectedItem) > 0)
        {
            SetIsOpen(*selectedItem, 1);
        }
        else if (ChildCount(*selectedItem) > 0)
            *selectedItem = GetChildAt(*selectedItem, 0);
    }
    if (movement == SelectionBox_Left)
    {
        if (IsOpen(*selectedItem))
        {
            SetIsOpen(*selectedItem, 0);
        }
        else if ((*selectedItem)->parent->parent)
            *selectedItem = (*selectedItem)->parent;
    }
    if (movement == SelectionBox_Down)
    {
        Item *itemBelow = GetItemBelow(*selectedItem);
        if (itemBelow)
            *selectedItem = itemBelow;
    }
    if (movement == SelectionBox_Up)
    {
        Item *itemAbove = GetItemAbove(*selectedItem);
        if (itemAbove && !IsRoot(itemAbove))
            *selectedItem = itemAbove;
    }
}
