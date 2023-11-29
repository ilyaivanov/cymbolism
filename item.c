#include "types.h"

void AssignChildren(Item *item, char **children, int childrenLen)
{
    item->childrenCount = childrenLen;
    item->children = calloc(childrenLen, sizeof(Item));
    for (int i = 0; i < childrenLen; i++)
    {
        Item *child = item->children + i;
        child->parent = item;
        child->text = *(children + i);
    }
}

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


    AssignChildren(root, items, ArrayLength(items));
    AssignChildren(root->children + 3, asuraChildren, ArrayLength(asuraChildren));
    AssignChildren((root->children + 3)->children, lostEdemItems, ArrayLength(lostEdemItems));
    AssignChildren((root->children + 3)->children + 3, lostEdemItems, ArrayLength(lostEdemItems));
    // AssignChildren(root->children + 17, solarFieldsChildren, ArrayLength(solarFieldsChildren));
    // AssignChildren(root->children + 18, syncChildren, ArrayLength(syncChildren));

}