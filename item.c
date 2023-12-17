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
        "Fred Brooks",
        "Good judgement comes from experience, and experience comes from bad judgement.",
        "The programmer, like the poet, works only slightly removed from pure thought-stuff. He builds his castles in the air, from air, creating by exertion of the imagination. Few media of creation are so flexible, so easy to polish and rework, so readily capable of realizing grand conceptual structures.",
        "Adding manpower to a late software project, makes it later.",
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
    AssignChildren(root->children + 3, asuraChildren, ArrayLength(asuraChildren));
    AssignChildren((root->children + 3)->children, lostEdemItems, ArrayLength(lostEdemItems));
    AssignChildren((root->children + 3)->children + 3, lostEdemItems, ArrayLength(lostEdemItems));
    AssignChildren(root->children + 4, quotes, ArrayLength(quotes));
    // AssignChildren(root->children + 17, solarFieldsChildren, ArrayLength(solarFieldsChildren));
    // AssignChildren(root->children + 18, syncChildren, ArrayLength(syncChildren));

}