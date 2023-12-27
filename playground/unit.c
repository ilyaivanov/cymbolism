#include <windows.h>
#include <stdio.h>
#include "..\memory.c"
#include "..\text.c"
#include "..\textReflow.c"

void main()
{
    FontData font = {0};
    for(int i = '0'; i <= 'z'; i++)
        font.textures[i].width = 1;

    Item item = {0};
    char *t = "012 456 890 234";
    InitBuffer(&item.textBuffer, t, strlen(t));

    u32 maxWidth = 4;
    SplitTextIntoLines(&item, &font, maxWidth);

    int expected[] = {0, 3, 7, 11, 15};

    for(int i = 0; i < ArrayLength(expected); i++)
    {
        if(item.newLines[i] != expected[i])
        {
            printf("New line different at %d. Expected: %d but was %d\n", i, expected[i], item.newLines[i]);
        }
    }

    printf("Done\n");
}