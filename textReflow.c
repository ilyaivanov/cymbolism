#include "types.h"

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
            int w = font->textures[ch].width;
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