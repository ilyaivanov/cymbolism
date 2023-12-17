#include "types.h"

void AssignChildren(Item *item, char **children, int childrenLen)
{
    item->childrenCount = childrenLen;
    item->children = calloc(childrenLen, sizeof(Item));
    item->isOpen = 1;
    for (int i = 0; i < childrenLen; i++)
    {
        Item *child = item->children + i;
        child->parent = item;


        char *text = *(children + i);
        i32 textLen = strlen(text) + 1;
        child->textBuffer.length = textLen;
        child->textBuffer.capacity = textLen * 2;

        child->textBuffer.text = malloc(child->textBuffer.capacity);
        CopyMemory(child->textBuffer.text, text, textLen + 1);
    }
}

// inline void MoveBytesForward(char *ptr, int length) {
//     for (int i = length - 1; i > 0; i--) {
//         ptr[i] = ptr[i - 1];
//     }
// }

// void InsertCharAt(Item* item, i32 at, i32 ch){
//     item->textBuffer.length += 1;
//     if(item->textBuffer.length >= item->textBuffer.capacity)
//     {
//         char *currentStr = item->textBuffer.text;
//         item->textBuffer.capacity *= 2;
//         item->textBuffer.text = malloc(item->textBuffer.capacity);
//         CopyMemory(item->textBuffer.text, currentStr, item->textBuffer.length);
//         free(currentStr);
//     }

//     MoveBytesForward(item->textBuffer.text + at, item->textBuffer.length - at);
//     *(item->textBuffer.text + at) = ch;
// }

void InitRoot(Item * root){
    char *items[] = {
        "Carbon Based Lifeforms",
        "Circular",
        "I Awake",
        "James Murray",
        "Miktek",
        "Koan",
        "Zero Cult",
        "Androcell",
        "Scann-Tec",
        "Hol Baumann",
        "Asura",
        "Cell",
        "Biosphere",
        // "Aes Dana",
        // "Side Liner",
        // "Fahrenheit Project",
        // "H.U.V.A Network",
        // "Solar Fields",
        // "Sync24",
    };

    char *asuraChildren[] = {
        "Lost Eden",
        "360",
        "Life2",
        "Code Eternity"};

    char *solarFieldsChildren[] = {
        "Reflective Frequencies",
        "Leaving Home",
    };
    char *syncChildren[] = {
        "Omnious",
        "Comfortable Void",
    };

    char *lostEdemItems[] = {
        "One 1",
        "One 2",
        "One 3",
        "One 4",
    };

    root->isOpen = 1;
    AssignChildren(root, items, ArrayLength(items));
    AssignChildren(root->children + 3, asuraChildren, ArrayLength(asuraChildren));
    AssignChildren((root->children + 3)->children, lostEdemItems, ArrayLength(lostEdemItems));
    AssignChildren((root->children + 3)->children + 3, lostEdemItems, ArrayLength(lostEdemItems));
    // AssignChildren(root->children + 17, solarFieldsChildren, ArrayLength(solarFieldsChildren));
    // AssignChildren(root->children + 18, syncChildren, ArrayLength(syncChildren));

}