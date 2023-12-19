#include "types.h"

inline void MoveBytesRight(char *ptr, int length) 
{
    for (int i = length - 1; i > 0; i--) {
        ptr[i] = ptr[i - 1];
    }
}
inline void MoveBytesLeft(char *ptr, int length) 
{
    for (int i = 0; i < length - 1; i++) {
        ptr[i] = ptr[i + 1];
    }
}

//TODO: need to extract this filtering code away from here
void InitBuffer(StringBuffer *buffer, char* text, i32 sourceSize)
{
    buffer->capacity = sourceSize * 2;
    buffer->text = AllocateMemory(buffer->capacity);
    i32 targetIndex = 0;
    for (int sourceIndex = 0; sourceIndex < sourceSize; sourceIndex++)
    {
        char ch = *(text + sourceIndex);
        if (ch != '\r')
        {
            *(buffer->text + targetIndex) = ch;
            targetIndex++;
        }
    }
    buffer->length = targetIndex;
    // CopyMemory(buffer->text, text, buffer->length);
}

void InsertCharAt(StringBuffer *buffer, i32 at, i32 ch)
{
    if(buffer->length >= buffer->capacity)
    {
        char *currentStr = buffer->text;
        buffer->capacity *= 2;
        buffer->text = AllocateMemory(buffer->capacity);
        CopyMemory(buffer->text, currentStr, buffer->length);
        FreeMemory(currentStr);
    }

    buffer->length += 1;
    MoveBytesRight(buffer->text + at, buffer->length - at);
    *(buffer->text + at) = ch;
}

void RemoveCharAt(StringBuffer *buffer, i32 at)
{
    MoveBytesLeft(buffer->text + at, buffer->length - at);
    buffer->length--;
}

i32 ParseUTF8Codepoint(u8 *bytes, i32 i, u32 *codepoint)
{
    if (!(bytes[i] & 0b10000000))
    {
        *codepoint = (u32)bytes[i];
        return 1;
    }
    else if ((bytes[i] & 0b11100000) == 0b11000000)
    {
        *codepoint = ((u32)(bytes[i] & 0b00011111) << 6) | (u32)(bytes[i + 1] & 0b00111111);
        return 2;
    }
    else if ((bytes[i] & 0b11110000) == 0b11100000)
    {
        u32 p1 = (u32)(bytes[i] & 0b00001111);
        u32 p2 = (u32)(bytes[i + 1] & 0b00111111);
        u32 p3 = (u32)(bytes[i + 2] & 0b00111111);

        *codepoint = p1 << 12 | p2 << 6 | p3;
        return 3;
    }
    else if ((bytes[i] & 0b11111000) == 0b11110000)
    {
        u32 p1 = (u32)(bytes[i] & 0b00000111);
        u32 p2 = (u32)(bytes[i + 1] & 0b00111111);
        u32 p3 = (u32)(bytes[i + 2] & 0b00111111);
        u32 p4 = (u32)(bytes[i + 3] & 0b00111111);

        *codepoint = p1 << 18 | p2 << 12 | p3 << 6 | p4;
        return 4;
    }

    MyAssert(0);
    return -1;
}