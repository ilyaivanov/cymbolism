#include "types.h"

typedef void ForEachLeveledHandler(AppState *state, Item * item, i32 level);

inline Item* GetChildAt(Item *parent, i32 index)
{
    return *(parent->children + index);
}

//TODO: check macros if function call would be too slow
//This will be moved inside UI model
inline void ForEachVisibleChild(AppState *state, Item *parent, ForEachLeveledHandler *handler)
{
    ItemInStack stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = parent->childrenCount - 1; c >= 0; c--)
        stack[++currentItemInStack] = (ItemInStack){GetChildAt(parent, c), 0};

    while (currentItemInStack >= 0)
    {
        ItemInStack current = stack[currentItemInStack--];
        Item *item = current.ref;

        handler(state, current.ref, current.level);

        if (item->isOpen)
        {
            for (int c = item->childrenCount - 1; c >= 0; c--)
                stack[++currentItemInStack] = (ItemInStack){GetChildAt(item, c), current.level + 1};
        }
    }
}


typedef void ForEachHandler(AppState *state, Item * item);
inline void ForEachActualChild(AppState *state, Item *parent, ForEachHandler *handler)
{
    Item *stack[512] = {0};
    int currentItemInStack = -1;

    for (int c = parent->childrenCount - 1; c >= 0; c--)
        stack[++currentItemInStack] = GetChildAt(parent, c);

    while (currentItemInStack >= 0)
    {
        Item *item = stack[currentItemInStack--];

        handler(state, item);

        for (int c = item->childrenCount - 1; c >= 0; c--)
            stack[++currentItemInStack] = GetChildAt(item, c);
    }
}


i32 GetItemIndex(Item *item)
{
    Item *parent = item->parent;
    for (int i = 0; i < parent->childrenCount; i++)
    {
        if (GetChildAt(parent, i) == item)
            return i;
    }
    return -1;
}


Item *GetItemBelow(Item *item)
{
    if (item->isOpen)
    {
        return *item->children;
    }
    else
    {
        Item *parent = item->parent;
        i32 itemIndex = GetItemIndex(item);
        if (itemIndex < parent->childrenCount - 1)
        {
            return GetChildAt(parent, itemIndex + 1);
        }
        else
        {
            while (parent->parent && GetItemIndex(parent) == parent->parent->childrenCount - 1 && parent->isOpen)
                parent = parent->parent;
            if (parent->parent)
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
    while(prevItem->isOpen)
        prevItem = GetChildAt(prevItem, prevItem->childrenCount - 1);
    return prevItem;

    return 0;
}

void AssignChildren(Item *item, char **childrenTexts, int childrenLen)
{
    item->childrenCount = childrenLen;
    Item **children = AllocateZeroedMemory(childrenLen, sizeof(Item*));
    item->children =  children;
    item->isOpen = 1;
    for (int i = 0; i < childrenLen; i++)
    {
        Item *child = AllocateZeroedMemory(1, sizeof(Item)); 
        *(children + i) = child;
        child->parent = item;

        char *text = *(childrenTexts + i);
        i32 textLen = strlen(text) + 1;
        child->textBuffer.length = textLen;
        child->textBuffer.capacity = textLen * 2;

        child->textBuffer.text = AllocateMemory(child->textBuffer.capacity);
        CopyMemory(child->textBuffer.text, text, textLen + 1);
    }
}

void FreeItem(AppState *state, Item *item)
{
    FreeMemory(item->textBuffer.text);
    if(item->childrenCount > 0)
        FreeMemory(item->children);
    FreeMemory(item);
}

Item* RemoveItem(AppState *state, Item *item)
{
    // TODO: before commiting this I need to document what changes to the memory I'm making
    // Moving Items around breaks parent links. I need to malloc individual Items and move references around
    // Design a tests which whould emulate an extensive work with items and monitor how memory is being used

    ForEachActualChild(state, item, FreeItem);
    FreeItem(state, item);
    
    i32 index = GetItemIndex(item);
    if(index < item->parent->childrenCount - 1)
    {
        memmove(item->parent->children + index, item->parent->children + index + 1, sizeof(Item*) * (item->parent->childrenCount - index));
    }

    item->parent->childrenCount--;

    if(item->parent->childrenCount == 0)
        return item->parent;
    if(index > 0)
        return GetChildAt(item->parent, index - 1);
    else 
        return GetChildAt(item->parent, index);
}




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
        "Fred Brooks",
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

    char *quotes[] = {
        "Good judgement comes from experience, and experience comes from bad judgement.",
        "The programmer, like the poet, works only slightly removed from pure thought-stuff. He builds his castles in the air, from air, creating by exertion of the imagination. Few media of creation are so flexible, so easy to polish and rework, so readily capable of realizing grand conceptual structures.",
        "Adding manpower to a late software project, makes it later.",
        "Systems program building is an entropy-decreasing process, hence inherently metastable. Program maintenance is an entropy-increasing process, and even its most skillful execution only delays the subsidence of the system into unfixable obsolescence.",
        "The bearing of a child takes nine months, no matter how many women are assigned.",
        "As time passes, the system becomes less and less well-ordered. Sooner or later the fixing cease to gain any ground. Each forward step is matched by a backward one. Although in principle usable forever, the system has worn out as a base for progress. A brand-new, from-the-ground-up redesign is necessary.",
        "A baseball manager recognizes a nonphysical talent, hustle, as an essential gift of great players and great teams. It is the characteristic of running faster than necessary, moving sooner than necessary, trying harder than necessary. It is essential for great programming teams, too.",
        "The general tendency is to over-design the second system, using all the ideas and frills that were cautiously sidetracked on the first one.",
        "All programmers are optimists",
        "The management question, therefore, is not whether to build a pilot system and throw it away. You will do that. The only question is whether to plan in advance to build a throwaway, or to promise to deliver the throwaway to customers.",
        "The programmer, like the poet, works only slightly removed from pure thought-stuff. He builds his castles in the air, from air, creating by exertion of the imagination.",
        "Einstein repeatedly argued that there must be simplified explanations of nature, because God is not capricious or arbitrary. No such faith comforts the software engineer.",
        "The challenge and the mission are to find real solutions to real problems on actual schedules with available resources.",
        "For the human makers of things, the incompletenesses and inconsistencies of our ideas become clear only during implementation.",
        "Men and months are interchangeable commodities only when a task can be partitioned among many workers with no communication among them (Fig. 2.1). This is true of reaping wheat or picking cotton; it is not even approximately true of systems programming.",
        "By documenting a design, the designer exposes himself to the criticisms of everyone, and he must be able to defend everything he writes. If the organizational structure is threatening in any way, nothing is going to be documented until it is completely defensible.",
        "The conclusion is simple: if a 200-man project has 25 managers who are the most competent and experienced programmers, fire the 175 troops and put the managers back to programming.",
        "An omelette, promised in two minutes, may appear to be progressing nicely. But when it has not set in two minutes, the customer has two choices—wait or eat it raw. Software customers have had the same choices. The cook has another choice; he can turn up the heat. The result is often an omelette nothing can save—burned in one part, raw in another.",
        "Organizations which design systems are constrained to produce systems which are copies of the communication structures of these organizations.",
        "Conceptual integrity in turn dictates that the design must proceed from one mind, or from a very small number of agreeing resonant minds.",
        "Today I am more convinced than ever. Conceptual integrity is central to product quality. Having a system architect is the most important single step toward conceptual integrity. These principles are by no means limited to software systems, but to the design of any complex construct, whether a computer, an airplane, a Strategic Defense Initiative, a Global Positioning System. After teaching a software engineering laboratory more than 20 times, I came to insist that student teams as small as four people choose a manager and a separate architect. Defining distinct roles in such small teams may be a little extreme, but I have observed it to work well and to contribute to design success even for small teams.",
        "A basic principle of data processing teaches the folly of trying to maintain independent files in synchonism.",
        "In fact, flow charting is more preached than practiced. I have never seen an experienced programmer who routinely made detailed flow charts before beginning to write programs",
        "Observe that for the programmer, as for the chef, the urgency of the patron may govern the scheduled completion of the task, but it cannot govern the actual completion.",
        "Adding manpower to a late software project makes it later.",
    };

    root->isOpen = 1;
    AssignChildren(root, items, ArrayLength(items));
    AssignChildren(*(root->children + 3), asuraChildren, ArrayLength(asuraChildren));
    // AssignChildren(*(root->children + 3)->children, lostEdemItems, ArrayLength(lostEdemItems));
    // AssignChildren(*(root->children + 3)->children + 3, lostEdemItems, ArrayLength(lostEdemItems));

    Item *brooks = *(root->children + 4);
    AssignChildren(brooks, quotes, ArrayLength(quotes));
    // brooks->isOpen = 0;

    // for (int i = 0; i < 8; i++)
    //     AssignChildren(root->children + 4 + i, quotes, ArrayLength(quotes));
}